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

#include <VistaBase/VistaStreamUtils.h>

using namespace std;

const static string g_sServerName = "localhost";
const static string g_sInputFilePath = "gershwin-mono.wav";
const static int g_iServerPort = 12480;
const static double g_dSampleRate = 44100;
const static int g_iBlockLength = 1024;
const static int g_iChannels = 300;

class CServer : public VistaThread
{
public:
	inline CServer( const string& sInputFilePath )
	{
		pInputFile = new ITAFileDatasource( sInputFilePath, g_iBlockLength );
		assert( pInputFile->GetNumberOfChannels() == 1 );
		pMuliplier = new ITAStreamMultiplier1N( pInputFile, g_iChannels );
		pStreamingServer = new CITANetAudioStreamingServer;

		pStreamingServer->SetInputStream( pMuliplier );

		Run();
	};

	inline ~CServer( )
	{
		delete pInputFile;
		delete pMuliplier;
		delete pStreamingServer;
	};

	void ThreadBody( )
	{
		vstr::out() << "[ NetAudioTestServer ] Starting net audio server and waiting for client connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
		pStreamingServer->Start( g_sServerName, g_iServerPort );		
	};

private:
	ITAFileDatasource* pInputFile;
	ITAStreamMultiplier1N* pMuliplier;
	CITANetAudioStreamingServer* pStreamingServer;
};

int main( int, char** )
{
	// Sample server (forked away into a thread)
	CServer oServer( g_sInputFilePath );

	// Client dumping received stream and mixing down to two channels
	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBlockLength, 100 * g_iBlockLength );

	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels() );
	for ( int i = 0; i < N ; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );
	
	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.stream.wav" );

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBlockLength );
	ITAPA.Initialize();
	ITAPA.SetPlaybackDatasource( &oProbe );
	ITAPA.Open();
	ITAPA.Start();

	vstr::out() << "[ NetAudioTestClient ] Waiting 1 second (net audio stream not connected and playing back zeros)" << endl;
	ITAPA.Sleep( 1.0f );

	vstr::out() << "[ NetAudioTestClient ] Will now connect to net audio server '" << g_sServerName << "' on port " << g_iServerPort << endl;
	try
	{
		if ( !oNetAudioStream.Connect( g_sServerName, g_iServerPort ) )
			ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to net audio server" );
	}
	catch ( ITAException e )
	{
		vstr::warn() << "[ NetAudioTestClient ] Connection failed." << endl;
		vstr::err() << e << endl;
		return 255;
	}
	vstr::out() << "[ NetAudioTestClient ] Connected." << endl;

	// Playback
	float fSeconds = 5.0f;
	vstr::out() << "[ NetAudioTestClient ] Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	vstr::out() << "[ NetAudioTestClient ] Done." << endl;

	vstr::out() << "[ NetAudioTestClient ] Will now disconnect from net audio server '" << g_sServerName << "' and port " << g_iServerPort << endl;
	vstr::out() << "[ NetAudioTestClient ] Closing in 1 second (net audio stream not connected and playing back zeros)" << endl;
	ITAPA.Sleep( 1.0f );

	ITAPA.Stop( );
	ITAPA.Close( );
	ITAPA.Finalize( );

	return 0;
}
