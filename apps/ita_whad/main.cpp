//#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

#include <ITAAsioInterface.h>
#include <ITAException.h>
#include <ITAPortaudioInterface.h>

using namespace std;

int main( int argc, char* argv[] )
{
	cout << "ita_whad - ITA's which audio devices application" << endl;
	cout << " ... lists all available audio devices and available properties." << endl << endl;
#ifdef ITA_WHAD_WITH_ASIO
	cout << "ASIO support: yes" << endl;
#else
	cout << "ASIO support: no" << endl;
#endif // ITA_WHAD_WITH_ASIO
#ifdef ITA_WHAD_WITH_PORTAUDIO
	cout << "Portaudio support: yes" << endl;
#else
	cout << "Portaudio support: no" << endl;
#endif // ITA_WHAD_WITH_PORTAUDIO
	cout << endl;

	stringstream ss;

#ifdef ITA_WHAD_WITH_ASIO

	ss.clear();
	ss << "[ASIO]" << endl << endl;

	ITAsioInitializeLibrary();
 
	long lDrivers = ITAsioGetNumDrivers();

	if( lDrivers == 0 )
		cerr << "Warning: no ASIO drivers found." << endl;

	for( long i = 0; i < lDrivers; i++ )
	{
		ss << "Driver identifier: " << ITAsioGetDriverName( i ) << endl;
		try
		{
			ASIOError e = ITAsioInitializeDriver( i );

			if( e != ASE_OK )
				ITA_EXCEPT1( INVALID_PARAMETER, "Could not initialize ASIO driver, audio device not connected?" );

			long lMinSize, lMaxSize, lPrefSize, g;
			ITAsioGetBufferSize( &lMinSize, &lMaxSize, &lPrefSize, &g );
			ss << "Buffer sizes: " << lMinSize << " " << lMaxSize << " " << lPrefSize << " " << g << endl;

			ASIOSampleRate fs;
			ITAsioGetSampleRate( &fs );
			ss << "Samplerate: " << fs << endl;

			long in, out;
			ITAsioGetChannels( &in, &out );
			ss << "Input channels: " << in << endl;
			ss << "Output channels: " << out << endl;

		}
		catch( const ITAException& e )
		{
			ss << "### ERROR: " << e << endl;
		}

		ss << endl;
	}

	ITAsioFinalizeLibrary();

	// Export
	string sFileNameASIO = "ita_whad_asio.txt";
	FILE* file_asio = fopen( sFileNameASIO.c_str(), "w" );
	fwrite( ss.str().c_str(), sizeof( char ), ss.str().length(), file_asio );
	fwrite( "\n", sizeof( char ), 1, file_asio );
	fclose( file_asio );
	cout << "Exported information to file '" << sFileNameASIO << "'" << endl << endl;

#endif // ITA_WHAD_WITH_ASIO

#ifdef ITA_WHAD_WITH_PORTAUDIO

	ss.clear();
	ss << "[Portaudio]" << endl << endl;

	ITAPortaudioInterface oPortaudioFirst( 44100.0f, 512 );
	oPortaudioFirst.Initialize();
	int iNumDevices = oPortaudioFirst.GetNumDevices();
	int iPeferredDefaultBufferSize = oPortaudioFirst.GetPreferredBufferSize();
	double dPreferredDefaultSampleRate = oPortaudioFirst.GetSampleRate();

	if( iNumDevices == 0 )
		cerr << "Warning: no Portaudio drivers found." << endl;

	std::vector< double > vdPreferredSampleRates( iNumDevices );
	for( int i = 0; i < iNumDevices; i++ )
	{
		oPortaudioFirst.GetDriverSampleRate( i, vdPreferredSampleRates[ i ] );
	}

	ss << "Default input device: " << oPortaudioFirst.GetDeviceName( oPortaudioFirst.GetDefaultInputDevice() ) << endl;
	ss << "Default output device: " << oPortaudioFirst.GetDeviceName( oPortaudioFirst.GetDefaultOutputDevice() ) << endl;

	oPortaudioFirst.Finalize();
		

	for( int i = 0; i < iNumDevices; i++ )
	{
		
		try
		{
			ITAPortaudioInterface oPA( vdPreferredSampleRates[ i ], iPeferredDefaultBufferSize );

			ITAPortaudioInterface::ITA_PA_ERRORCODE e = oPA.Initialize( i );
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ss << "Driver number: " << i + 1 << endl;
			ss << "Driver identifier: " << oPA.GetDeviceName( i ) << endl;

			e = oPA.Open();
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ss << "Latency: " << oPA.GetDeviceLatency( i ) << endl;

			double dSampleRate;
			e = oPA.GetDriverSampleRate( i, dSampleRate );
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ss << "Sample rate: " << dSampleRate << endl;

			int iIn, iOut;
			oPA.GetNumChannels( i, iIn, iOut );
			ss << "Input channels: " << iIn << endl;
			ss << "Output channels: " << iOut << endl;

			oPA.Close();
			oPA.Finalize();
			
		}
		catch( const ITAException& e )
		{
			ss << "### ERROR: " << e << endl;
		}

		ss << endl;
	}


	// Export
	string sFileNamePortaudio = "ita_whad_portaudio.txt";
	FILE* file_pa = fopen( sFileNamePortaudio.c_str(), "w" );
	fwrite( ss.str().c_str(), sizeof( char ), ss.str().length(), file_pa );
	fwrite( "\n", sizeof( char ), 1, file_pa );
	fclose( file_pa );
	cout << "Exported information to file '" << sFileNamePortaudio << "'" << endl << endl;

#endif // ITA_WHAD_WITH_PORTAUDIO

	return 0;
}
