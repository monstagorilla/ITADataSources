#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAFileDatasource.h>
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchbay.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44100;
static int g_iBlockLength = 1024;
static int g_iChannels = 2;

class CServer : public VistaThread
{
public:
	CServer( )
	{
		pGenerator = new ITAStreamFunctionGenerator( 2, g_dSampleRate, g_iBlockLength, ITAStreamFunctionGenerator::SINE, 456.78f, 0.81f, true );
		pDatei = new ITAFileDatasource( "gershwin-mono.wav", g_iBlockLength );
		pMuliplier = new ITAStreamMultiplier1N( pDatei, g_iChannels );
		pStreamingServer = new CITANetAudioStreamingServer;

		pStreamingServer->SetInputStream( pMuliplier );

		cout << "[ Server ] Starting server and waiting for connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;

		pStreamingServer->SetInputStream( pMuliplier );
	};
	~CServer( )
	{
		delete pGenerator;
		delete pDatei;
		delete pMuliplier;
		delete pStreamingServer;
	};
	void ThreadBody( )
	{
		pStreamingServer->Start( g_sServerName, g_iServerPort );

		
	};
private:
	ITAStreamFunctionGenerator *pGenerator;
	ITAFileDatasource* pDatei;
	ITAStreamMultiplier1N* pMuliplier;
	CITANetAudioStreamingServer* pStreamingServer;
};

int main( int, char** )
{
	// Server-Kram
	CServer oServer;

	// Client-Kram
	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBlockLength, 100 * g_iBlockLength );

	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	ITADatasource* pOutput;

	oPatchbay.AddOutput( 1 );
	/*
	for ( int i = 0; i < oNetAudioStream.GetNumberOfChannels( ); i++ )
	{
	if ( i % 2 == 0 )
	oPatchbay.ConnectChannels( 0, i, 0, 0 );
	else
	oPatchbay.ConnectChannels( 0, i, 0, 1 );

	*/
	oPatchbay.ConnectChannels( 0, 0, 0, 0, 1.0f );
	pOutput = oPatchbay.GetOutputDatasource( 0 );

	ITAStreamProbe oProbe( pOutput, "output.wav" );
	ITAStreamMultiplier1N oMultiplier( &oProbe, 2 );

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBlockLength );
	ITAPA.Initialize( );
	ITAPA.SetPlaybackDatasource( &oMultiplier );
	ITAPA.Open( );
	ITAPA.Start( );

	cout << "[ Client ] Waiting 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 3.0f );

	cout << "[ Client ] Will now connect to '" << g_sServerName << "' on port " << g_iServerPort << endl;
	try
	{
		if ( !oNetAudioStream.Connect( g_sServerName, g_iServerPort ) )
			ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to server" );
	}
	catch ( ITAException e )
	{
		cout << "[ Client ] Connection failed." << endl;
		cerr << e << endl;
		return 255;
	}
	cout << "[ Client ] Connected." << endl;

	// Playback
	float fSeconds = 10.0f;
	cout << "[ Client ] Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	cout << "[ Client ] Done." << endl;


	cout << "[ Client ] Will now disconnect from '" << g_sServerName << "' and port " << g_iServerPort << endl;
	cout << "[ Client ] Closing in 3 seconds (net audio stream not connected and returning zeros)" << endl;
	ITAPA.Sleep( 3.0f );

	ITAPA.Stop( );
	ITAPA.Close( );
	ITAPA.Finalize( );



	return 0;
}
