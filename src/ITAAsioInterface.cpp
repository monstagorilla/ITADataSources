#include <ITAAsioInterface.h>

#include <ITAClock.h>
#include <ITADatasource.h>
#include <ITADatasourceRealization.h>
#include <ITAException.h>
#include <ITAStopWatch.h>
#include <ITAStreamInfo.h>
#include <ITAStringUtils.h>
#include <atomic>
#include <host/asiodrivers.h>
#include <iostream>
#include <vector>
#include <windows.h>


// Puffer-Vielfachheit (Doppelpuffer, Dreifachpuffer, etc.)
const unsigned int ASIOSOURCE_BUFFER_MULTIPLICITY = 16;

class ASIOSource : public ITADatasourceRealization
{
public:
	inline ASIOSource( unsigned int uiChannels, double dSamplerate, unsigned int uiBlocklength )
	    : ITADatasourceRealization( uiChannels, dSamplerate, uiBlocklength, ASIOSOURCE_BUFFER_MULTIPLICITY ) { };

	friend ASIOTime *bufferSwitchTimeInfo( ASIOTime *timeInfo, long index, ASIOBool processNow );
};

// WICHTIG: Dieses Makro legt fest, wieviele Blöcke Eingangsdaten nach
//          dem Start des Streamings durch ASIOStart() verworfen werden.
#define NUM_INPUT_BLOCKS_TO_DISCARD 2

// WICHTIG: Dieses Makro legt fest, wieviele Blöcke Nulldaten nach
//          dem Stoppen des Streamings durch ASIOStop() noch abgespielt werden.
#define NUM_ZERO_BLOCKS_AFTER_STOP 2

#define DEBUG_OUT 0


// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#	define ASIO64toDouble( a ) ( a )
#else
const double twoRaisedTo32 = 4294967296.;
#	define ASIO64toDouble( a ) ( ( a ).lo + ( a ).hi * twoRaisedTo32 )
#endif

typedef struct DriverInfo
{
	// Zugehörige ASIO-Treiberinfo (Achtung: Datentyp "ASIODriverInfo")
	ASIODriverInfo driverInfo;

	// Anzahl der Kanäle
	long inputChannels;
	long outputChannels;

	// Puffergrößen
	long minSize;
	long maxSize;
	long preferredSize;
	long granularity;

	// Samplerate
	ASIOSampleRate sampleRate;

	// ASIOOutputReady()
	bool postOutput;

	// Latenzen
	long inputLatency;
	long outputLatency;

	// Anzahl der Ein-/Ausgabepuffer
	long inputBuffers;  // becomes number of actual created input buffers
	long outputBuffers; // becomes number of actual created output buffers
	//	ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's
	std::vector<ASIOBufferInfo> bufferInfos; // buffer info's

	// ASIOGetChannelInfo()
	//	ASIOChannelInfo channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
	std::vector<ASIOChannelInfo> channelInfos; // channel info's
	// The above two arrays share the same indexing, as the data in them are linked together

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double nanoSeconds;
	double samples;
	double tcSamples; // time code samples

	// bufferSwitchTimeInfo()
	ASIOTime tInfo;           // time info state
	unsigned long sysRefTime; // system reference time, when bufferSwitch() was called

	// Signal the end of processing in this example
	bool stopped;
} DriverInfo;

// Statusflags
const long LIBRAW      = -1; // Bibliothek wurde noch nicht initialisiert
const long LOADED      = 0;  // Bibliothek wurde initialisiert
const long INITIALIZED = 1;  // ASIO-Treiber wurde initialisiert
const long PREPARED    = 2;  // ASIO-Streaming wurde vorbereitet
const long RUNNING     = 3;  // ASIO-Streaming läuft

HANDLE hStopEvent = 0;       // Event, welches "Bereit zum Stoppen" signalisiert
CRITICAL_SECTION csInternal; // Kritischer Bereich: Interne Synchronisation
CRITICAL_SECTION csExternal; // Kritischer Bereich: Externe Synchronisation

long lState   = LIBRAW;        // Aktueller Zustand
bool bPreInit = false;         // Treiber-Komponente initialisiert und aufgezählt?
int iZeroBlocks;               // Anzahl der noch zu spielenden Nullblöcke (nach Stop)
long lNumDrivers;              // Anzahl der installierten Treiber
char **asioDriverNames = NULL; // Zwischerspeicher für die Treibernamen
DriverInfo asioDriverInfo;     // Eigene Treiberinfos
float fOutputGain = 1.0;       // Verstärkungsfaktor für Wiedergabe
long volatile lAsioBufferIndex;
ASIOBool abProcessNow;

float *pfSilence          = 0;
float *pfInputBuffer      = 0;
float *pfInputDummyBuffer = 0; // Puffer für die zu verwerfenden ersten zwei Blöcke

ASIOSource *pasInputDatasource      = 0;                                   // Eingabedatenquelle
ITADatasource *pidsOutputDatasource = 0;                                   // Ausgabedatenquelle
ITAStreamInfo siStreamInfo;                                                // Zustandsinformationen des Streams
double g_dStreamStartTimeStamp = ITAClock::getDefaultClock( )->getTime( ); //!< Time stamp at beginning of streaming

unsigned long ulOutputBlockCounter  = 0;
unsigned long ulInputCounter        = 0;
unsigned long ulInputPresentCounter = 0;
int iInputBufferNrOffset            = 0;

long lBuffersize = 0; // Ausgewählte Puffergröße

std::atomic<int> iBufferswitchEntrances( 0 );

ITAStopWatch g_swStreamOutputProcessing;

// -= Externe Referenzen =-
extern AsioDrivers *asioDrivers;

/*
   +------------------+
   |                  |
   |  ASIO-Callbacks  |
   |                  |
   +------------------+
   */

ASIOCallbacks asioCallbacks;

ASIOTime *bufferSwitchTimeInfo( ASIOTime *timeInfo, long index, ASIOBool processNow )
{
	// The actual processing callback.
	/* Beware that this is normally in a seperate thread,
	   hence be sure that you take care about thread synchronization.
	   This is omitted here for simplicity. */

	if( ++iBufferswitchEntrances > 1 )
		std::cerr << "Problem: ASIO bufferswitch callback reentrance!\n" << std::endl;

	// Internen Mutex in Besitz bringen
	EnterCriticalSection( &csInternal );

	// Store the timeInfo for later use
	asioDriverInfo.tInfo = *timeInfo;
	lAsioBufferIndex     = index;
	abProcessNow         = processNow;

	static long processedSamples = 0;

	// Get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if( timeInfo->timeInfo.flags & kSystemTimeValid )
		asioDriverInfo.nanoSeconds = ASIO64toDouble( timeInfo->timeInfo.systemTime );
	else
		asioDriverInfo.nanoSeconds = 0;

	if( timeInfo->timeInfo.flags & kSamplePositionValid )
		asioDriverInfo.samples = ASIO64toDouble( timeInfo->timeInfo.samplePosition );
	else
		asioDriverInfo.samples = 0;

	if( timeInfo->timeCode.flags & kTcValid )
		asioDriverInfo.tcSamples = ASIO64toDouble( timeInfo->timeCode.timeCodeSamples );
	else
		asioDriverInfo.tcSamples = 0;

	// Get the system reference time
	asioDriverInfo.sysRefTime = timeGetTime( ); // link winmm.lib

	/*
	    DEBUG_PRINTF("[ITAsioInterface] nanoSeconds = %0.1f, samples = %0.1f, tcSamples = %0.1f, sysRefTime = %0.1f\n",
	    asioDriverInfo.nanoSeconds, asioDriverInfo.samples, asioDriverInfo.tcSamples, asioDriverInfo.sysRefTime);
	    */

	// [Bugfix fwe]: Schwerer Bug! long buffSize = asioDriverInfo.preferredSize;
	int j;

	float fScaling     = 0;
	int iChannelNumber = 0;

	// -= Eingabedaten lesen =-

	/* Hinweis: Eingabedaten werden nur gelesen, falls ASIO-Stop noch
	            nicht aufgerufen wurde, d.h. iZeroBlocks == -1 ist. */
	if( iZeroBlocks == -1 )
	{
		bool bInputDataPresent = ( ulInputPresentCounter >= NUM_INPUT_BLOCKS_TO_DISCARD ) && ( pasInputDatasource != 0 );

		// Input-Kanaele lesen
		for( int i = 0; i < asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++ )
		{
			if( asioDriverInfo.bufferInfos[i].isInput == TRUE )
			{
				if( i >= asioDriverInfo.inputBuffers )
					MessageBoxA( 0, "Error", "Input Buffer Overflow", MB_OK | MB_SYSTEMMODAL | MB_APPLMODAL );
				else
					iChannelNumber = i;

				if( !bInputDataPresent )
				{
					// Noch keine Eingabesamples: Stille bereistellen
					float *pfBuffer = pasInputDatasource->GetWritePointer( iChannelNumber );
					for( j = 0; j < lBuffersize; j++ )
						pfBuffer[j] = 0;
				}
				else
				{
					// Eingabesamples verfügbar. Samples konvertieren.
					pfInputBuffer = pasInputDatasource->GetWritePointer( iChannelNumber );

					switch( asioDriverInfo.channelInfos[i].type )
					{
						case ASIOSTInt16LSB:
							__int16 *psAsioBufPtr;
							psAsioBufPtr = (__int16 *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];

							// Konvertierung Int16 nach Float
							for( j = 0; j < lBuffersize; j++ )
								pfInputBuffer[j] = ( (float)psAsioBufPtr[j] ) / 32767.0F;
							break;

						case ASIOSTInt24LSB: // used for 20 bits as well
							char *pcAsioBufPtr;
							pcAsioBufPtr = (char *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];
							__int32 i32Temp;
							i32Temp = 0;

							// Konvertierung 20/24-Bit Integer nach Float
							for( j = 0; j < lBuffersize; j++ )
							{
								memcpy( &i32Temp, pcAsioBufPtr, 3 );
								pcAsioBufPtr += 3;
								pfInputBuffer[j] = ( (float)i32Temp ) / 8388607.0F;
							}
							break;

						case ASIOSTInt32LSB:
							__int32 *piAsioBufPtr;
							piAsioBufPtr = (__int32 *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];

							/*
							 *  Bugfix [fwe 2008-09-24]:
							 *
							 *  Bei der Benutzung der Hammerfall fiel uns auf das
							 *  diese nicht den vollständigen 32-Bit Dynamikbereich unterstützt
							 *  (Karte arbeitet mit 24-Bit). Es scheint als müßte des
							 *  Least Significant Byte (Bits 0-7) immer Null sein.
							 *  (Entspricht wieder den 24 Bit nominell). Demnach ist
							 *  der Skalierungsfaktor für Vollaussteuerung nicht
							 *  2^31-1 = 2147483647, sondern (2^23-1)*2^8 = 2147483392
							 *
							 *  Klärung mit RME Audio steht noch aus...
							 */

							// Konvertierung 32-Bit Integer nach Float
							for( j = 0; j < lBuffersize; j++ )
								// Standard-Implementierung:
								// pfInputBuffer[j] = ((float) piAsioBufPtr[j]) / 2147483647.0F;
								// RME-Hammerfall Safe Implementierung
								pfInputBuffer[j] = ( (float)piAsioBufPtr[j] ) / 2147483392.0F;

							break;
					}
				}
			}
		}

		if( pasInputDatasource )
			pasInputDatasource->IncrementWritePointer( );
	}

	// -= Ausgabedaten schreiben =-

	/* Hinweis: Falls iZeroBlocks != -1 ist wurde ASIOStop bereits ausgelöst und
	            jetzt müssen Nullblöcke abgespielt werden.
	            iZeroBlocks muß heruntergezählt werden. */

	// Provide ouput data

	g_swStreamOutputProcessing.start( );
	for( int i = 0; i < asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++ )
	{
		if( asioDriverInfo.bufferInfos[i].isInput == FALSE )
		{
			if( i - asioDriverInfo.inputBuffers >= asioDriverInfo.outputBuffers )
				MessageBoxA( 0, "Error", "Output Buffer Overflow", MB_OK | MB_SYSTEMMODAL | MB_APPLMODAL );
			else
				iChannelNumber = i - asioDriverInfo.inputBuffers;

			const float *pfOutputData = pfSilence;

			// TODO: Streaminfo zusammenstellen? TimeCode aus ASIO rein?

			// Gibts eine Ausgabedatenquelle?
			if( pidsOutputDatasource && ( iZeroBlocks == -1 ) )
			{
				// Dann Datenzeiger der Quelle abrufen
				// Wichtig: false ist enorm wichtig, damit nicht auf Daten gewartet wird!
				pfOutputData = pidsOutputDatasource->GetBlockPointer( (unsigned int)iChannelNumber, &siStreamInfo );

				// Falls die Quelle keine Daten hat, Stille verwenden
				if( !pfOutputData )
					pfOutputData = pfSilence;
			}

			switch( asioDriverInfo.channelInfos[i].type )
			{
				case ASIOSTInt16LSB:
#if DEBUG_OUT
					std::cout << "ASIOSTInt16LSB nicht unterstuetzt ";
#endif
					__int16 *psAsioBufPtr;
					psAsioBufPtr = (__int16 *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];
					fScaling     = 32767.0F * fOutputGain;
					for( j = 0; j < lBuffersize; j++ )
						psAsioBufPtr[j] = (__int16)( pfOutputData[j] * fScaling );
					break;

				case ASIOSTInt24LSB: // used for 20 bits as well
#if DEBUG_OUT
					std::cout << "ASIOSTInt24LSB nicht unterstuetzt ";
#endif
					char *pcAsioBufPtr;
					pcAsioBufPtr = (char *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];
					__int32 i32Temp;
					char *pcSourcePointer;
					fScaling = 8388607.0F * fOutputGain;
					for( j = 0; j < lBuffersize; j++ )
					{
						i32Temp         = (__int32)( pfOutputData[j] * fScaling );
						pcSourcePointer = (char *)&i32Temp;
						memcpy( pcAsioBufPtr, pcSourcePointer, 3 );
						pcAsioBufPtr += 3;
					}
					break;

				case ASIOSTInt32LSB:
#if DEBUG_OUT
					std::cout << "ASIOSTInt32LSB nicht unterstuetzt";
#endif

					/*
					 *  Bugfix [fwe 2008-09-24]:
					 *
					 *  Bei der Benutzung der Hammerfall fiel uns auf das
					 *  diese nicht den vollständigen 32-Bit Dynamikbereich unterstützt
					 *  (Karte arbeitet mit 24-Bit). Es scheint als müßte des
					 *  Least Significant Byte (Bits 0-7) immer Null sein.
					 *  (Entspricht wieder den 24 Bit nominell). Demnach ist
					 *  der Skalierungsfaktor für Vollaussteuerung nicht
					 *  2^31-1 = 2147483647, sondern (2^23-1)*2^8 = 2147483392
					 *
					 *  Klärung mit RME Audio steht noch aus...
					 */

					__int32 *piAsioBufPtr;
					piAsioBufPtr = (__int32 *)asioDriverInfo.bufferInfos[i].buffers[lAsioBufferIndex];
					// fScaling = 2147483647.0F * fOutputGain;
					fScaling = 2147483392.0F * fOutputGain;
					for( j = 0; j < lBuffersize; j++ )
						piAsioBufPtr[j] = (__int32)( pfOutputData[j] * fScaling );
					break;

					// Alles weitere wird noch nicht unterstützt!!
					// Int
				case ASIOSTInt32LSB16: // 32 bit data with 18 bit alignment
					std::cout << "ASIOSTInt32LSB16 nicht unterstuetzt ";
					break;
				case ASIOSTInt32LSB18: // 32 bit data with 18 bit alignment
					std::cout << "ASIOSTInt32LSB18 nicht unterstuetzt ";
					break;
				case ASIOSTInt32LSB20: // 32 bit data with 20 bit alignment
					std::cout << "ASIOSTInt32LSB20 nicht unterstuetzt ";
					break;
				case ASIOSTInt32LSB24: // 32 bit data with 24 bit alignment
					std::cout << "ASIOSTInt32LSB24 nicht unterstuetzt ";
					break;
				case ASIOSTInt16MSB:
					std::cout << "ASIOSTInt16MSB nicht unterstuetzt ";
					break;
				case ASIOSTInt24MSB: // used for 20 bits as well
					std::cout << "ASIOSTInt24MSB nicht unterstuetzt ";
					break;
				case ASIOSTInt32MSB:
					std::cout << "ASIOSTInt32MSB nicht unterstuetzt ";
					break;

					// these are used for 32 bit data buffer, with different alignment of the data inside
					// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32MSB16: // 32 bit data with 18 bit alignment
					std::cout << "ASIOSTInt32MSB16 nicht unterstuetzt ";
					break;
				case ASIOSTInt32MSB18: // 32 bit data with 18 bit alignment
					std::cout << "ASIOSTInt32MSB18 nicht unterstuetzt ";
					break;
				case ASIOSTInt32MSB20: // 32 bit data with 20 bit alignment
					std::cout << "ASIOSTInt32MSB20 nicht unterstuetzt ";
					break;
				case ASIOSTInt32MSB24: // 32 bit data with 24 bit alignment
					std::cout << "ASIOSTInt32MSB24 nicht unterstuetzt ";
					break;

					// Float
				case ASIOSTFloat32LSB: // IEEE 754 32 bit float, as found on Intel x86 architecture
					std::cout << "ASIOSTFloat32LSB nicht unterstuetzt ";
					break;
				case ASIOSTFloat64LSB:                                  // IEEE 754 64 bit double float, as found on Intel x86 architecture
					std::cout << "ASIOSTFloat64LSB nicht unterstuetzt"; // these are used for 32 bit data buffer, with different alignment of the data inside
					break;                                              // 32 bit PCI bus systems can more easily used with these

				case ASIOSTFloat32MSB: // IEEE 754 32 bit float, as found on Intel x86 architecture
					std::cout << "ASIOSTFloat32MSB nicht unterstuetzt ";
					break;
				case ASIOSTFloat64MSB: // IEEE 754 64 bit double float, as found on Intel x86 architecture
					std::cout << "ASIOSTFloat64MSB nicht unterstuetzt";
					break;
				default:
					std::cout << "- ";
			}
		}
	}

	const double dOutputProcessingTime    = g_swStreamOutputProcessing.stop( );
	const double dAvailabelProcessingTime = double( lBuffersize ) / double( asioDriverInfo.sampleRate );
	if( dOutputProcessingTime > dAvailabelProcessingTime )
		std::cerr << "[ ITAAsioInterface ] Dropout detected, block processing time exceeded ( took " << timeToString( dOutputProcessingTime ) << " but got only "
		          << timeToString( dAvailabelProcessingTime ) << " )" << std::endl;

	ulOutputBlockCounter++;

	if( ulInputPresentCounter <= 1 )
		ulInputPresentCounter++;
	else
		ulInputCounter++;

	processedSamples += lBuffersize;

	siStreamInfo.nSamples += lBuffersize;
	// siStreamInfo.dStreamTimeCode = (double) (siStreamInfo.nSamples) / (double) asioDriverInfo.sampleRate;
	siStreamInfo.dStreamTimeCode = ITAClock::getDefaultClock( )->getTime( ) - g_dStreamStartTimeStamp;
	siStreamInfo.dSysTimeCode    = ITAClock::getDefaultClock( )->getTime( );

	// Gibts eine Ausgabedatenquelle? Dann Blockzeiger weitersetzen
	if( pidsOutputDatasource && ( iZeroBlocks == -1 ) )
		pidsOutputDatasource->IncrementBlockPointer( );

	// Stop triggert? Herunterzählen? Bei 0 das Event für wartendes ITAAsioStop() setzen.
	if( iZeroBlocks > 0 )
		if( ( --iZeroBlocks ) == 0 )
			SetEvent( hStopEvent );

	// Internen Mutex freigeben
	LeaveCriticalSection( &csInternal );

	// Finally if the driver supports the ASIOOutputReady() optimization,
	// do it here, all data are in place
	// if (asioDriverInfo.postOutput)
	ASIOOutputReady( ); // Muss hier stehen, sonst spielts nicht

	--iBufferswitchEntrances;

	return 0L;
}

void bufferSwitch( long index, ASIOBool processNow )
{ // the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that
	// you take care about thread synchronization. This is omitted here for
	// simplicity.

	// _RPTF0( _CRT_WARN, "Callback BufferSwitch \n");

	// As this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs
	// to be created though it will only set the timeInfo.samplePosition and
	// timeInfo.systemTime fields and the according flags
	ASIOTime timeInfo;
	memset( &timeInfo, 0, sizeof( timeInfo ) );

	// Get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo( &timeInfo, index, processNow );
}

void sampleRateChanged( ASIOSampleRate )
{
	// MessageBox (0, "Callback sampleRateChanged", "Callback sampleRateChanged", MB_OK|MB_SYSTEMMODAL| MB_APPLMODAL );

	// Do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
}


long asioMessages( long selector, long value, void *, double * )
{
	// MessageBox (0, "Callback asioMessages", "Callback asioMessages", MB_OK|MB_SYSTEMMODAL| MB_APPLMODAL );

	// Currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch( selector )
	{
		case kAsioSelectorSupported:
			if( value == kAsioResetRequest || value == kAsioEngineVersion || value == kAsioResyncRequest ||
			    value == kAsioLatenciesChanged
			    // the following three were added for ASIO 2.0, you don't necessarily have to support them
			    || value == kAsioSupportsTimeInfo || value == kAsioSupportsTimeCode || value == kAsioSupportsInputMonitor )
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			asioDriverInfo.stopped; // In this sample the processing will just stop
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:

			// MessageBox (0, "Callback sampleRateChanged BEN", "Callback sampleRateChanged BEN ", MB_OK|MB_SYSTEMMODAL| MB_APPLMODAL );

			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}

/*
   +-----------------------------------------------+
   |                                               |
   |  Implementierung der öffentlichen Funktionen  |
   |                                               |
   +-----------------------------------------------+
   */

ITASIO_API void ITAsioInitializeLibrary( void )
{
	if( lState != LIBRAW )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO library already initialized" );

	InitializeCriticalSection( &csInternal );
	InitializeCriticalSection( &csExternal );

	if( FAILED( hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ) )
		ITA_EXCEPT1( UNKNOWN, "Failed to create mutex" );

	// ASIO-Umgebung ermitteln
	asioDrivers = new AsioDrivers( );

	// Setzen der Asio-Callbacks
	asioCallbacks.bufferSwitch         = &bufferSwitch;
	asioCallbacks.sampleRateDidChange  = &sampleRateChanged;
	asioCallbacks.asioMessage          = &asioMessages;
	asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

	// Treibernamen ermitteln und zwischenspeichern
	lNumDrivers = asioDrivers->asioGetNumDev( );
	if( lNumDrivers > 0 )
	{
		asioDriverNames = new char *[lNumDrivers];
		memset( asioDriverNames, 0, lNumDrivers * sizeof( char * ) );
		for( int i = 0; i < (int)lNumDrivers; i++ )
			asioDriverNames[i] = new char[32];

		asioDrivers->getDriverNames( asioDriverNames, lNumDrivers );
	}

	// Startzustand setzen
	lState = LOADED;
}

ITASIO_API void ITAsioFinalizeLibrary( void )
{
	// Gar nicht initialisiert? => Kein Fehler
	if( lState == LIBRAW )
		return;

	if( lNumDrivers > 0 )
	{
		for( int i = 0; i < (int)lNumDrivers; i++ )
			delete[] asioDriverNames[i];
		delete[] asioDriverNames;
		asioDriverNames = NULL;
	}

	// Treiber noch initialisiert? => Implizit abrüumen
	if( lState > LOADED )
		ITAsioFinalizeDriver( );

	// ASIO-Umgebung und Resourcen freigeben
	delete asioDrivers;

	CloseHandle( hStopEvent );

	DeleteCriticalSection( &csInternal );
	DeleteCriticalSection( &csExternal );

	lState = LIBRAW;
}

/*
#define REQUIRE_LIBINIT()    { if (lState == LIBRAW) ITA_EXCEPT1(MODAL_EXCEPTION, "ASIO library not initialized"); }
#define REQUIRE_DRVINIT()    { if (lState < INITIALIZED) ITA_EXCEPT1(MODAL_EXCEPTION, "No ASIO driver initialized"); }
#define REQUIRE_STREAMPREP() { if (lState < PREPARED) ITA_EXCEPT1(MODAL_EXCEPTION, "ASIO streaming not prepared"); }
*/

ITASIO_API long ITAsioGetNumDrivers( void )
{
	if( lState < LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO library not initialized" );
	return lNumDrivers;
}

ITASIO_API const char *ITAsioGetDriverName( long lDriverNr )
{
	if( lState < LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO library not initialized" );

	// BUGFIX [fwe, 2007-06-24]: Leerer String != NULL
	static const char EMPTY_STRING = '\0';

	// Leeren String zurückgeben, falls ungültiger Index
	if( ( lDriverNr < 0 ) || ( lDriverNr >= lNumDrivers ) )
		return &EMPTY_STRING;
	return asioDriverNames[lDriverNr];
}

ITASIO_API const char *ITAsioGetErrorStr( ASIOError ae )
{
	switch( ae )
	{
		case ASE_NotPresent: // hardware input or output is not present or available
			return "Hardware is not present or available";

		case ASE_HWMalfunction: // hardware is malfunctioning (can be returned by any ASIO function)
			return "Hardware is malfunctioning";

		case ASE_InvalidParameter: // input parameter invalid
			return "Invalid input parameter";

		case ASE_InvalidMode: // hardware is in a bad mode or used in a bad mode
			return "Hardware is in a bad mode or used in a bad mode";

		case ASE_SPNotAdvancing: // hardware is not running when sample position is inquired
			return "Hardware is not running when sample position is inquired";

		case ASE_NoClock: // sample clock or rate cannot be determined or is not present
			return "Sample clock or rate cannot be determined or is not present";

		case ASE_NoMemory: // not enough memory for completing the request
			return "Not enough memory for completing the request";
		default:
			return "";
	}
}

ITASIO_API ASIOError ITAsioInitializeDriver( long lDriverNr )
{
	if( lState < LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO library not initialized" );

	if( lState > LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO driver already initialized" );

	if( ( lDriverNr < 0 ) || ( lDriverNr >= lNumDrivers ) )
		return ( ASE_InvalidParameter );
	return ITAsioInitializeDriver( asioDriverNames[lDriverNr] );
}

ITASIO_API ASIOError ITAsioInitializeDriver( const char *pszDriverName )
{
	if( lState < LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO library not initialized" );

	if( lState > LOADED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO driver already initialized" );

	// Mutex in Besitz bringen
	EnterCriticalSection( &csExternal );

	// Load the driver, this will setup all the necessary internal data structures
	try
	{
		bool bLoadSuccess = asioDrivers->loadDriver( (char *)pszDriverName );
		if( !bLoadSuccess )
			return ASE_NotPresent;
	}
	catch( ... )
	{
		return ASE_NotPresent;
	}

	ASIOError aeResult = ASIOInit( &asioDriverInfo.driverInfo );
	if( aeResult != ASE_OK )
	{
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	lState = INITIALIZED;

	// Hier sollen alle Inhalte der Get-Funktionen rein,
	// um später im Hauptprogramm die Struktur auf einmal abfragen zu können !!!

	// TODO: Frage von fwefers: Sind die Sleeps nötig? Warum?

	aeResult = ASIOGetChannels( &asioDriverInfo.inputChannels, &asioDriverInfo.outputChannels );
	if( aeResult != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	Sleep( 50 );

	if( ( aeResult = ASIOGetLatencies( &asioDriverInfo.inputLatency, &asioDriverInfo.outputLatency ) ) != ASE_OK )
	{
		/* Continue upon latency resolve failure (happens with Focusrite but is not harmful)
		ASIOExit();
		lState = LOADED;
		LeaveCriticalSection(&csExternal);
		return aeResult;
		*/
		asioDriverInfo.inputLatency  = 0;
		asioDriverInfo.outputLatency = 0;
	}
	Sleep( 50 );

	aeResult = ASIOGetBufferSize( &asioDriverInfo.minSize, &asioDriverInfo.maxSize, &asioDriverInfo.preferredSize, &asioDriverInfo.granularity );
	if( aeResult != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	Sleep( 50 );

	aeResult = ASIOGetSampleRate( &asioDriverInfo.sampleRate );
	if( aeResult != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	Sleep( 50 );

	ASIOClockSource ClockSourceArray[3];
	long numSources = 3;
	aeResult        = ASIOGetClockSources( ClockSourceArray, &numSources );
	if( aeResult != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	// Retrieve total number of channels
	long lNumInChannels, lNumOutChannels;
	if( ( aeResult = ASIOGetChannels( &lNumInChannels, &lNumOutChannels ) ) != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	// Prepare channel info array
	size_t nNumInOutChannels = size_t( lNumInChannels + lNumOutChannels );
	asioDriverInfo.channelInfos.resize( nNumInOutChannels );

	if( nNumInOutChannels == 0 )
		return ASE_InvalidParameter;

	// Retrieve channel info
	ASIOChannelInfo *pInfo = reinterpret_cast<ASIOChannelInfo *>( &asioDriverInfo.channelInfos.front( ) );
	if( ( aeResult = ASIOGetChannelInfo( pInfo ) ) != ASE_OK )
	{
		ASIOExit( );
		lState = LOADED;
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API ASIOError ITAsioFinalizeDriver( void )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	// Mutex in Besitz bringen
	EnterCriticalSection( &csExternal );

	// Ggf. DisposeBuffers implizieren!
	if( lState > INITIALIZED )
		ITAsioDisposeBuffers( );

	ASIOError aeResult = ASIOExit( );

	if( aeResult == ASE_OK )
		lState = LOADED;
	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API ASIOError ITAsioGetBufferSize( long *minSize, long *maxSize, long *preferredSize, long *granularity )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	// TODO: Frage von fwefers: Warum werden die Werte oben zwischengespeichert
	//                          und hier nochmal ausgelesen (DriverInfo)?
	ASIOError aeResult           = ASIOGetBufferSize( minSize, maxSize, preferredSize, granularity );
	asioDriverInfo.minSize       = *minSize;
	asioDriverInfo.maxSize       = *maxSize;
	asioDriverInfo.preferredSize = *preferredSize;
	asioDriverInfo.granularity   = *granularity;
	return aeResult;
}

ITASIO_API ASIOError ITAsioGetChannels( long *numInputChannels, long *numOutputChannels )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	ASIOError aeResult            = ASIOGetChannels( numInputChannels, numOutputChannels );
	asioDriverInfo.inputChannels  = *numInputChannels;
	asioDriverInfo.outputChannels = *numOutputChannels;
	return aeResult;
}

ITASIO_API ASIOError ITAsioGetLatencies( long *inputLatency, long *outputLatency )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	ASIOError aeResult           = ASIOGetLatencies( inputLatency, outputLatency );
	asioDriverInfo.inputLatency  = *inputLatency;
	asioDriverInfo.outputLatency = *outputLatency;
	return aeResult;
}

ITASIO_API ASIOError ITAsioCreateBuffers( long lNumberInputChannels, long lNumberOutputChannels, long lBufferSize )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );
	if( lState > INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming already prepared" );

	// Mutex in Besitz bringen
	EnterCriticalSection( &csExternal );

	// BUGFIX [fwe, 2007-06-24] Aufruf von CreateBuffers entfernt die gesetzte Datenquelle
	pidsOutputDatasource = NULL;

	asioDriverInfo.inputBuffers  = lNumberInputChannels;
	asioDriverInfo.outputBuffers = lNumberOutputChannels;

	const long numChannels = asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers;
	int i, j;

	asioDriverInfo.bufferInfos.resize( numChannels );

	for( i = 0; i < asioDriverInfo.inputBuffers; i++ )
	{
		asioDriverInfo.bufferInfos[i].buffers[0] = 0;
		asioDriverInfo.bufferInfos[i].buffers[1] = 0;
		asioDriverInfo.bufferInfos[i].channelNum = i;
		// True, wenn Input, sonst False
		asioDriverInfo.bufferInfos[i].isInput = ASIOTrue;
	}

	for( j = 0; j < asioDriverInfo.outputBuffers; j++ )
	{
		asioDriverInfo.bufferInfos[i + j].buffers[0] = 0;
		asioDriverInfo.bufferInfos[i + j].buffers[1] = 0;
		asioDriverInfo.bufferInfos[i + j].channelNum = j;
		// True, wenn Input, sonst False
		asioDriverInfo.bufferInfos[i + j].isInput = ASIOFalse;
	}

	ASIOError aeResult = ASIOCreateBuffers( reinterpret_cast<ASIOBufferInfo *>( &asioDriverInfo.bufferInfos.front( ) ), numChannels, lBufferSize, &asioCallbacks );
	if( aeResult != ASE_OK )
	{
		LeaveCriticalSection( &csExternal );
		return aeResult;
	}

	asioDriverInfo.channelInfos.resize( numChannels );

	for( i = 0; i < ( asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers ); i++ )
	{
		// Testen, ob man das in den oberen Schleifen erledigen kann,
		// oder ob GetChannelInfo erst nach CreateBuffer funktioniert
		asioDriverInfo.channelInfos[i].channel = asioDriverInfo.bufferInfos[i].channelNum;
		asioDriverInfo.channelInfos[i].isInput = asioDriverInfo.bufferInfos[i].isInput;
		ASIOError aeResult                     = ASIOGetChannelInfo( &asioDriverInfo.channelInfos[i] );
		if( aeResult != ASE_OK )
		{
			LeaveCriticalSection( &csExternal );
			return aeResult;
		}
	}

	lBuffersize = lBufferSize;

	// Eigene Buffer erzeugen
	if( lNumberOutputChannels > 0 )
	{
		pfSilence = new float[lBufferSize];
		for( long k = 0; k < lBufferSize; k++ )
			pfSilence[k] = 0;
	}

	// Eingabedatenquelle erzeugen
	// TODO: Fehlerbehandlung
	if( lNumberInputChannels > 0 )
	{
		pasInputDatasource = new ASIOSource( (unsigned int)asioDriverInfo.inputBuffers, (double)asioDriverInfo.sampleRate, (unsigned int)lBufferSize );
		// Eingabe-Dummy-Puffer erzeugen
		pfInputDummyBuffer = new float[lBufferSize];
	}

	lState = PREPARED;

	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API ASIOError ITAsioDisposeBuffers( void )
{
	if( lState < PREPARED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming has not been prepared" );

	// Mutex in Besitz bringen
	EnterCriticalSection( &csExternal );

	// Ggf. ASIOStop implizieren!
	if( lState > PREPARED )
		ITAsioStop( );

	ASIOError aeResult = ASIODisposeBuffers( );

	// Datenquelle verwerfen
	delete pasInputDatasource;
	pasInputDatasource = 0;

	// Eigene Buffer freigeben
	delete[] pfInputDummyBuffer;
	pfInputDummyBuffer = 0;
	delete[] pfSilence;
	pfSilence = 0;

	lBuffersize = 0;

	lState = INITIALIZED;

	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API void ITAsioSetGain( double dGain )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	EnterCriticalSection( &csInternal );
	// Wert auf das Intervall [-1.0, +1.0] begrenzen
	fOutputGain = ( dGain < 0.0 ? 0.0F : (float)dGain );
	LeaveCriticalSection( &csInternal );
}

ITASIO_API ASIOError ITAsioCanSampleRate( ASIOSampleRate sampleRate )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	return ASIOCanSampleRate( sampleRate );
}

ITASIO_API ASIOError ITAsioGetSampleRate( ASIOSampleRate *currentRate )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	return ASIOGetSampleRate( currentRate );
}

ITASIO_API ASIOError ITAsioSetSampleRate( ASIOSampleRate sampleRate )
{
	if( lState < INITIALIZED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "No ASIO driver initialized" );

	// BUGFIX [fwe, 2007-06-24]: Interne Aktualisierung der Samplerate
	ASIOError aeResult = ASIOSetSampleRate( ASIOSampleRate( sampleRate ) );
	if( aeResult == ASE_OK )
		asioDriverInfo.sampleRate = sampleRate;
	return aeResult;
}

ITASIO_API ASIOError ITAsioStart( void )
{
	if( lState < PREPARED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming has not been prepared" );

	if( lState > PREPARED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming is already running" );

	// Mutex in Besitz bringen
	EnterCriticalSection( &csExternal );

	// Update stream start time stamp
	g_dStreamStartTimeStamp = ITAClock::getDefaultClock( )->getTime( );

	ulInputPresentCounter = 0;
	ulInputCounter        = 0;
	ulOutputBlockCounter  = 0;

	siStreamInfo.nSamples        = 0;
	siStreamInfo.dStreamTimeCode = 0;
	siStreamInfo.dSysTimeCode    = ITAClock::getDefaultClock( )->getTime( );

	// Alle Eingangsdatenquellen zurücksetzen
	if( asioDriverInfo.inputBuffers > 0 )
		pasInputDatasource->Reset( );

	iZeroBlocks = -1;
	ResetEvent( hStopEvent );

	ASIOError aeResult = ASIOStart( );
	if( aeResult == ASE_OK )
		lState = RUNNING;

	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API ASIOError ITAsioStop( void )
{
	if( lState < RUNNING )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming is not running" );

	EnterCriticalSection( &csExternal );

	// Zunächst die Nullblöcke spielen und warten bis diese abgespielt wurden
	// [stienen] Hier wartet der Loopback Test bis unendlich! Wer hat das hInternalMutex und will es nicht hergeben?!
	EnterCriticalSection( &csInternal );
	iZeroBlocks = NUM_ZERO_BLOCKS_AFTER_STOP;
	LeaveCriticalSection( &csInternal );

	/* Wichtig: Hier muss ein TIME_OUT angegeben werden!
	            Falls der ASIO-Thread durch einen Crash stirbt,
	            würden wir hier bis an alle Ewigkeit warten...
	            Deshalb Timeout von 1s (das sollte ausreichen) */
	WaitForSingleObject( hStopEvent, 1000 );

	// Stop signalisieren
	ASIOError aeResult = ASIOStop( );

	const double dAvailableProcessingTime = double( lBuffersize ) / double( asioDriverInfo.sampleRate );
	if( g_swStreamOutputProcessing.cycles( ) && g_swStreamOutputProcessing.maximum( ) > dAvailableProcessingTime )
		std::cout << "[ ITAAsioInterface ] Detected dropouts and presenting output processing statistics: " << g_swStreamOutputProcessing.ToString( ) << std::endl;

	Sleep( 100 ); // <-- Sicherheits Waitstate (Aus ASIO-Beispiel von Steinberg)

	EnterCriticalSection( &csInternal );
	if( aeResult == ASE_OK )
		lState = PREPARED;
	LeaveCriticalSection( &csInternal );

	LeaveCriticalSection( &csExternal );

	return aeResult;
}

ITASIO_API ASIOError ITAsioSetPlaybackDatasource( ITADatasource *pidsDatasource )
{
	if( lState < PREPARED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming has not been prepared" );

	if( pidsDatasource )
	{
		// Parameter überprüfen
		if( (long)pidsDatasource->GetNumberOfChannels( ) != asioDriverInfo.outputBuffers )
			ITA_EXCEPT1( MODAL_EXCEPTION, "Datasource has invalid number of channels" );

		if( ASIOSampleRate( pidsDatasource->GetSampleRate( ) ) != asioDriverInfo.sampleRate )
			ITA_EXCEPT1( MODAL_EXCEPTION, "Datasource has invalid samplerate" );

		if( (long)pidsDatasource->GetBlocklength( ) != lBuffersize )
			ITA_EXCEPT1( MODAL_EXCEPTION, "Datasource has invalid block length" );
	}

	EnterCriticalSection( &csInternal );
	pidsOutputDatasource = pidsDatasource;
	LeaveCriticalSection( &csInternal );

	return ASE_OK;
}

ITASIO_API ITADatasource *ITAsioGetRecordDatasource( )
{
	if( lState < PREPARED )
		ITA_EXCEPT1( MODAL_EXCEPTION, "ASIO streaming has not been prepared" );
	return pasInputDatasource;
}

ITASIO_API ASIOError ITAsioControlPanel( void )
{
	return ASIOControlPanel( );
}

/*
   +-----------------------------+
   |                             |
   |  Einstiegsfunktion der DLL  |
   |                             |
   +-----------------------------+
   */

BOOL APIENTRY DllMain( HANDLE, DWORD fdwReason, LPVOID )
{
	/*
	   Wichtig: Durch "return FALSE" kommt es beim Starten der Client-Anwendung
	   zu einer Windows-Fehlermeldung, das die DLL nicht geladen werden
	   konnte und die Anwendung kann nicht gestartet werden!
	   */

	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			// Die DLL in den virtuellen Adressraum geladen.
			break;

		case DLL_PROCESS_DETACH:
			// Die DLL in den virtuellen Adressraum entfernt ("entladen").
			break;
	}

	return TRUE;
}
