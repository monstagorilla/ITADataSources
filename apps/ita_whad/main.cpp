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

#ifdef ITA_WHAD_WITH_ASIO

	stringstream ssa;

	ITAsioInitializeLibrary();
 
	long lDrivers = ITAsioGetNumDrivers();

	if( lDrivers == 0 )
		cerr << "Warning: no ASIO drivers found." << endl;

	for( long i = 0; i < lDrivers; i++ )
	{
		ssa << "Driver identifier: " << ITAsioGetDriverName( i ) << endl;
		try
		{
			ASIOError e = ITAsioInitializeDriver( i );

			if( e != ASE_OK )
				ITA_EXCEPT1( INVALID_PARAMETER, "Could not initialize ASIO driver, audio device not connected?" );

			long lMinSize, lMaxSize, lPrefSize, g;
			ITAsioGetBufferSize( &lMinSize, &lMaxSize, &lPrefSize, &g );
			ssa << "Buffer sizes: " << lMinSize << " " << lMaxSize << " " << lPrefSize << " " << g << endl;

			ASIOSampleRate fs;
			ITAsioGetSampleRate( &fs );
			ssa << "Samplerate: " << fs << endl;

			long in, out;
			ITAsioGetChannels( &in, &out );
			ssa << "Input channels: " << in << endl;
			ssa << "Output channels: " << out << endl;

		}
		catch( const ITAException& e )
		{
			ssa << "### ERROR: " << e << endl;
		}

		ssa << endl;
	}

	ITAsioFinalizeLibrary();

	// Export
	string sFileNameASIO = "ita_whad_asio.txt";
	FILE* file_asio = fopen( sFileNameASIO.c_str(), "w" );
	fwrite( ssa.str().c_str(), sizeof( char ), ssa.str().length(), file_asio );
	fwrite( "\n", sizeof( char ), 1, file_asio );
	fclose( file_asio );
	cout << "Exported information to file '" << sFileNameASIO << "'" << endl << endl;

#endif // ITA_WHAD_WITH_ASIO

#ifdef ITA_WHAD_WITH_PORTAUDIO

	stringstream ssp;
	
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

	ssp << "Default input device: " << oPortaudioFirst.GetDeviceName( oPortaudioFirst.GetDefaultInputDevice() ) << endl;
	ssp << "Default output device: " << oPortaudioFirst.GetDeviceName( oPortaudioFirst.GetDefaultOutputDevice() ) << endl;

	ssp << endl;

	oPortaudioFirst.Finalize();
		

	for( int i = 0; i < iNumDevices; i++ )
	{
		
		try
		{
			ITAPortaudioInterface oPA( vdPreferredSampleRates[ i ], iPeferredDefaultBufferSize );

			ITAPortaudioInterface::ITA_PA_ERRORCODE e = oPA.Initialize( i );
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ssp << "Driver number: " << i + 1 << endl;
			ssp << "Driver identifier: " << oPA.GetDeviceName( i ) << endl;

			e = oPA.Open();
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ssp << "Latency: " << oPA.GetDeviceLatency( i ) << endl;

			double dSampleRate;
			e = oPA.GetDriverSampleRate( i, dSampleRate );
			if( e != ITAPortaudioInterface::ITA_PA_NO_ERROR )
				ITA_EXCEPT1( INVALID_PARAMETER, "Portaudio error: " + ITAPortaudioInterface::GetErrorCodeString( e ) );

			ssp << "Sample rate: " << dSampleRate << endl;

			int iIn, iOut;
			oPA.GetNumChannels( i, iIn, iOut );
			ssp << "Input channels: " << iIn << endl;
			ssp << "Output channels: " << iOut << endl;

			oPA.Close();
			oPA.Finalize();
			
		}
		catch( const ITAException& e )
		{
			ssp << "### ERROR: " << e << endl;
		}

		ssp << endl;
	}


	// Export
	string sFileNamePortaudio = "ita_whad_portaudio.txt";
	FILE* file_pa = fopen( sFileNamePortaudio.c_str(), "w" );
	fwrite( ssp.str().c_str(), sizeof( char ), ssp.str().length(), file_pa );
	fwrite( "\n", sizeof( char ), 1, file_pa );
	fclose( file_pa );
	cout << "Exported information to file '" << sFileNamePortaudio << "'" << endl << endl;

#endif // ITA_WHAD_WITH_PORTAUDIO

	return 0;
}
