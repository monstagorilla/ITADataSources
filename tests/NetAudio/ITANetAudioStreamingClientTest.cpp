#include <iostream>
#include <string>

#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchbay.h>

using namespace std;

//static string g_sServerName = "137.226.61.163";
static string g_sServerName = "137.226.61.163";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44100;
static int g_iBufferSize = 2048;
static int g_iChannels = 150;

int main( int , char** )
{
	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBufferSize, 1 * g_iBufferSize );
	
	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBufferSize );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels( ) );
	for ( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );

	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.stream.wav" );


	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBufferSize );
	ITAPA.Initialize();
	ITAPA.SetPlaybackDatasource( &oProbe );
	ITAPA.Open();
	ITAPA.Start(); 

	cout << "Waiting 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 2.0f );

	cout << "Will now connect to '" << g_sServerName << "' on port " << g_iServerPort << endl;
	try
	{
		if( !oNetAudioStream.Connect( g_sServerName, g_iServerPort ) )
			ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to server" );
	}
	catch( ITAException e )
	{
		cout << "Connection failed." << endl;
		cerr << e << endl;
		return 255;
	}
	cout << "Connected." << endl;

	// Playback
	float fSeconds = 20.0f;
	cout << "Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	cout << "Done." << endl;


	cout << "Will now disconnect from '" << g_sServerName << "' and port " << g_iServerPort << endl;
	cout << "Closing in 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 1.0f );

	ITAPA.Stop();
	ITAPA.Close();
	ITAPA.Finalize();
	
	return 0;
}
