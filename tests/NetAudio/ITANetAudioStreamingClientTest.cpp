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

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44100;
static int g_iBufferSize = 1024;

int main( int , char** )
{
	CITANetAudioStream oNetAudioStream( 10, g_dSampleRate, g_iBufferSize, 100 * g_iBufferSize );
	ITAStreamProbe oProbe( &oNetAudioStream, "out_gutentag.wav" );
	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBufferSize );
	oPatchbay.AddInput( &oProbe );
	ITADatasource* pOutput;
	oPatchbay.AddOutput( 1 );
	for ( int n = 0; n<1; n++ )
	{
		if ( oProbe.GetNumberOfChannels( ) > 1 )
			oPatchbay.ConnectChannels( 0, n % ( oProbe.GetNumberOfChannels( ) - 1 ), 0, n );
		else
			oPatchbay.ConnectChannels( 0, 0, 0, n );
	}
	pOutput = oPatchbay.GetOutputDatasource( 0 );
	std::cout << "NumOutChannel " << pOutput->GetNumberOfChannels() << std::endl;
	
	
	//ITAStreamMultiplier1N oMultiplier( &oProbe, 2 );

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBufferSize );
	ITAPA.Initialize();
	ITAPA.SetPlaybackDatasource( pOutput );
	ITAPA.Open();
	ITAPA.Start(); 

	cout << "Waiting 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 3.0f );

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
	float fSeconds = 100.0f;
	cout << "Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	cout << "Done." << endl;


	cout << "Will now disconnect from '" << g_sServerName << "' and port " << g_iServerPort << endl;
	cout << "Closing in 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 3.0f );

	ITAPA.Stop();
	ITAPA.Close();
	ITAPA.Finalize();
	
	

	return 0;
}
