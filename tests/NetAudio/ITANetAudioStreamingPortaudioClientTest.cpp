#include <iostream>
#include <string>
#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchbay.h>
using namespace std;

int main(int argc, char* argv[])
{
	
	if (argc != 7)
	{
		cout << "argc = " << argc << endl;
		cout << "sServerName = " << argv[1] << endl;
		cout << "iServerPort = " << argv[2] << endl;
		cout << "dSampleRate = " << argv[3] << endl;
		cout << "iBlockLength = " << argv[4] << endl;
		cout << "iChannels = " << argv[5] << endl;
		cout << "iBufferSize = " << argv[6] << endl;
		fprintf(stderr, "Fehler: Syntax = ServerName ServerPort SampleRate BufferSize Channel RingBufferSize!\n");
	}	

	string sServerName = argv[1];
	unsigned int iServerPort = atoi(argv[2]);
	double dSampleRate = strtod(argv[3], NULL);
	int iBlockLength = atoi(argv[4]);
	int iChannels = atoi(argv[5]);
	int iBufferSize = atoi(argv[6]);
	
	CITANetAudioStream oNetAudioStream( iChannels, dSampleRate, iBlockLength, iBlockLength*16 );
	oNetAudioStream.SetDebuggingEnabled( true );
	ITAStreamPatchbay oPatchbay( dSampleRate, iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );
	int N = int( oNetAudioStream.GetNumberOfChannels( ) );
	for ( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );
	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.stream.wav" );
	ITAPortaudioInterface ITAPA( dSampleRate, iBufferSize );
	ITAPA.Initialize();
	ITAPA.SetPlaybackDatasource( &oNetAudioStream );
	ITAPA.Open();
	ITAPA.Start(); 
	cout << "Waiting 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 2.0f );
	cout << "Will now connect to '" << sServerName << "' on port " << iServerPort << endl;
	try
	{
		if( !oNetAudioStream.Connect( sServerName, iServerPort ) )
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
	float fSeconds = 10.0f; // 15min
	cout << "Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	cout << "Done." << endl;
	cout << "Will now disconnect from '" << sServerName << "' and port " << iServerPort << endl;
	cout << "Closing in 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 1.0f );
	ITAPA.Stop();
	ITAPA.Close();
	ITAPA.Finalize();
	
	return 0;
}
