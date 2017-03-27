#include <cmath>
#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchBay.h>

#include <VistaBase/VistaStreamUtils.h>

using namespace std;

const static string g_sServerName = "localhost";
const static string g_sInputFilePath = "gershwin-mono.wav";
const static int g_iServerPort = 12480;
const static double g_dSampleRate = 44100;
const static int g_iBlockLength = 512;
const static int g_iChannels = 2;
const static int g_iTargetLatencySamples = g_iBlockLength * 3;
const static int g_iRingerBufferCapacity = g_iBlockLength * 4;

class CServer : public VistaThread
{
public:
	inline CServer( const string& sInputFilePath )
	{
		pStreamingServer = new CITANetAudioStreamingServer;
		pStreamingServer->SetTargetLatencySamples( g_iTargetLatencySamples );
		pStreamingServer->SetServerLogBaseName( "ITANetAudioTest_Server" );

		pInputFile = new ITAFileDatasource( sInputFilePath, g_iBlockLength );
		assert( pInputFile->GetNumberOfChannels() == 1 );
		pMuliplier = new ITAStreamMultiplier1N( pInputFile, g_iChannels );
		pInputStreamProbe = new ITAStreamProbe( pMuliplier, "ITANetAudioTest.serverstream.wav" );
		pStreamingServer->SetInputStream( pInputStreamProbe );

		Run();
	};

	inline ~CServer( )
	{
		delete pInputFile;
		delete pMuliplier;
		delete pStreamingServer;
		delete pInputStreamProbe;
	};

	void ThreadBody( )
	{
		vstr::out() << "[ NetAudioTestServer ] Starting net audio server and waiting for client connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
		pStreamingServer->Start( g_sServerName, g_iServerPort, 0.1 );		
	};

private:
	ITAFileDatasource* pInputFile;
	ITAStreamMultiplier1N* pMuliplier;
	CITANetAudioStreamingServer* pStreamingServer;
	ITAStreamProbe* pInputStreamProbe;

};

int main( int, char** )
{
	// Sample server (forked away into a thread)
	CServer* pServer = new CServer( g_sInputFilePath );

	// Client dumping received stream and mixing down to two channels
	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBlockLength, g_iRingerBufferCapacity );
	oNetAudioStream.SetNetAudioStreamingLoggerBaseName( "ITANetAudioTest_Client" );

	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels() );
	for ( int i = 0; i < N ; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );
	
	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.netstream.wav" );

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
	float fSeconds = 10.0f;
	vstr::out() << "[ NetAudioTestClient ] Playback started, waiting " << fSeconds << " seconds" << endl;
	ITAPA.Sleep( fSeconds ); // blocking
	vstr::out() << "[ NetAudioTestClient ] Done." << endl;

	oNetAudioStream.Disconnect();

	vstr::out() << "[ NetAudioTestClient ] Will now disconnect from net audio server '" << g_sServerName << "' and port " << g_iServerPort << endl;
	vstr::out() << "[ NetAudioTestClient ] Closing in 1 second (net audio stream not connected and playing back zeros)" << endl;
	ITAPA.Sleep( 1.0f );

	ITAPA.Stop( );
	ITAPA.Close( );
	ITAPA.Finalize( );

	delete pServer;

	return 0;
}
