#include <ITADatasourceUtils.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamProbe.h>
#include <ITAFileDataSource.h>
#include <ITAException.h>
#include <ITAStringUtils.h>

#include <iostream>
#include <string>

using namespace std;

int main( int argc, char *argv[] )
{
	if( argc != 5 )
		ITA_EXCEPT1( INVALID_PARAMETER, "Syntax error: ITAPortaudioMeasurementTest.exe excitation.wav recording.wav samplerate blocklength" );
	
	try
	{
		std::string sExcitationSignalPath = std::string( argv[ 1 ] );
		std::string sRecordingSignalPath = std::string( argv[ 2 ] );
		double dSampleRate = StringToDouble( std::string( argv[ 3 ] ) );
		int iBlockLength = StringToInt( std::string( argv[ 4 ] ) );

		// Recording
		ITAPortaudioInterface ITAPA_IN( dSampleRate, iBlockLength );
		int iInputDevice = ITAPA_IN.GetDefaultInputDevice();;
		ITAPA_IN.Initialize( iInputDevice );
		std::string sInDevice = ITAPA_IN.GetDeviceName( iInputDevice );
		std::cout << "Input device identifier: " << sInDevice << endl;
		int iNumInputChannels, temp;
		ITAPA_IN.GetNumChannels( iInputDevice, iNumInputChannels, temp );

		ITAPA_IN.SetPlaybackEnabled( false );
		ITAPA_IN.SetRecordEnabled( true );
		ITAStreamProbe oRecordStreamProbe( ITAPA_IN.GetRecordDatasource(), sRecordingSignalPath );

		ITAPA_IN.Open();

		// Playback
		ITAPortaudioInterface ITAPA_OUT( dSampleRate, iBlockLength );
		int iOutputDevice = ITAPA_OUT.GetDefaultOutputDevice();;
		ITAPA_OUT.Initialize( iOutputDevice );
		std::string sOutDevice = ITAPA_OUT.GetDeviceName( iOutputDevice );
		std::cout << "Output device identifier: " << sOutDevice << endl;
		int iNumOutputChannels;
		ITAPA_OUT.GetNumChannels( iOutputDevice, temp, iNumOutputChannels );

		ITAFileDatasource oPlayback( sExcitationSignalPath, iBlockLength );

		// Start streaming
		ITAPA_OUT.SetRecordEnabled( false );
		ITAPA_OUT.SetPlaybackEnabled( true );
		ITAPA_OUT.SetPlaybackDatasource( &oPlayback );

		ITAPA_OUT.Open();

		ITAPA_IN.Start();
		ITAPA_OUT.Start();

		// Measurement
		WriteFromDatasourceToFile( &oRecordStreamProbe, sRecordingSignalPath, oPlayback.GetFileCapacity(), 1.0, true, true );

		// Stop streaming
		ITAPA_OUT.Stop();
		ITAPA_OUT.Close();
		ITAPA_OUT.Finalize();

		ITAPA_IN.Stop();
		ITAPA_IN.Close();
		ITAPA_IN.Finalize();
	}
	catch( const ITAException& e )
	{
		cerr << e << endl;
	}

	return 0;
}