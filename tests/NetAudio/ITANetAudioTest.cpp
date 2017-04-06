#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioSampleServer.h>
#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchBay.h>
#include <ITAAsioInterface.h>
#include <ITAStreamInfo.h>

#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaBase/VistaExceptionBase.h>

#include <cmath>
#include <iostream>
#include <string>

using namespace std;

const static string g_sServerName = "localhost";
const static string g_sInputFilePath = "gershwin-mono.wav";
const static int g_iServerPort = 12480;
const static double g_dSampleRate = 44100;
const static int g_iBlockLength = 512;
const static int g_iChannels = 2;
const static int g_iTargetLatencySamples = g_iBlockLength * 2;
const static int g_iRingerBufferCapacity = g_iBlockLength * 10;
const static double g_dDuration = 10.0f;
const static double g_dSyncTimout = 0.001f;
const static bool g_bUseASIO = true;
const static string g_sAudioInterface = "ASIO MADIface USB";
//const static string g_sAudioInterface = "ASIO Hammerfall DSP";
//const static string g_sAudioInterface = "ASIO4ALL v2";
const static bool g_bUseUDP = false;

class CSampleGenerator : public CITASampleProcessor
{
public:
	inline CSampleGenerator() 
		: CITASampleProcessor( g_dSampleRate, g_iChannels, g_iBlockLength )
	{};
	
	inline void Process( const ITAStreamInfo* pStreamInfo )
	{
		for( size_t c = 0; c < m_vvfSampleBuffer.size(); c++ )
		{
			for( size_t n = 0; n < m_vvfSampleBuffer[ c ].size(); n++ )
			{
				float fSample = ( c == 0 ? 1.0f : -1.0f ) * ( 1.0f - 2.0f * n / float( GetBlocklength() ) );
				m_vvfSampleBuffer[ c ][ n ] = fSample;
			}
		}
	};
};

class CSampleServerExample : public VistaThread
{
public:
	inline CSampleServerExample()
	{
		m_pSampleGenerator = new CSampleGenerator();
		m_pSampleServer = new CITANetAudioSampleServer( m_pSampleGenerator );
		m_pSampleServer->SetDebuggingEnabled( true );
		m_pSampleServer->SetTargetLatencySamples( g_iTargetLatencySamples );
		m_pSampleServer->SetServerLogBaseName( "ITANetAudioTest_Server" );

		Run();
	};

	inline ~CSampleServerExample()
	{
		delete m_pSampleServer;
		delete m_pSampleGenerator;
	};

	void ThreadBody()
	{
		vstr::out() << "[ NetAudioTestServer ] Starting net audio sample server and waiting for client connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
		m_pSampleServer->Start( g_sServerName, g_iServerPort, g_dSyncTimout, g_bUseUDP );
	};

	CITANetAudioSampleServer* m_pSampleServer;
	CSampleGenerator* m_pSampleGenerator;
};

class CStreamServerExample : public VistaThread
{
public:
	inline CStreamServerExample( const string& sInputFilePath )
	{
		m_pStreamingServer = new CITANetAudioStreamingServer;
		m_pStreamingServer->SetDebuggingEnabled( true );
		m_pStreamingServer->SetTargetLatencySamples( g_iTargetLatencySamples );
		m_pStreamingServer->SetServerLogBaseName( "ITANetAudioTest_Server" );

		m_pInputFile = new ITAFileDatasource( sInputFilePath, g_iBlockLength );
		m_pInputFile->SetIsLooping( true );
		assert( m_pInputFile->GetNumberOfChannels() == 1 );
		m_pMultiplier = new ITAStreamMultiplier1N( m_pInputFile, g_iChannels );
		m_pInputStreamProbe = new ITAStreamProbe( m_pMultiplier, "ITANetAudioTest.serverstream.wav" );
		m_pStreamingServer->SetInputStream( m_pInputStreamProbe );

		Run();
	};

	inline ~CStreamServerExample()
	{
		delete m_pInputFile;
		delete m_pMultiplier;
		delete m_pStreamingServer;
		delete m_pInputStreamProbe;
	};

	void ThreadBody( )
	{
		vstr::out() << "[ NetAudioTestServer ] Starting net audio server and waiting for client connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
		m_pStreamingServer->Start( g_sServerName, g_iServerPort, g_dSyncTimout, g_bUseUDP );
	};

private:
	ITAFileDatasource* m_pInputFile;
	ITAStreamMultiplier1N* m_pMultiplier;
	CITANetAudioStreamingServer* m_pStreamingServer;
	ITAStreamProbe* m_pInputStreamProbe;
};

void run_test()
{

	// Sample server (forked away into a thread)
	//CStreamServerExample* pServer = new CStreamServerExample( g_sInputFilePath );
	CSampleServerExample* pServer = new CSampleServerExample();

	// Client dumping received stream and mixing down to two channels
	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBlockLength, g_iRingerBufferCapacity );
	oNetAudioStream.SetNetAudioStreamingLoggerBaseName( "ITANetAudioTest_Client" );
	oNetAudioStream.SetDebuggingEnabled( true );

	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels() );
	for( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );

	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.netstream.wav" );

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBlockLength );
	if( g_bUseASIO )
	{
		ITAsioInitializeLibrary();
		ITAsioInitializeDriver( g_sAudioInterface );

		long lBuffersize, lDummy;
		ITAsioGetBufferSize( &lDummy, &lDummy, &lBuffersize, &lDummy );
		ITAsioSetSampleRate( ( ASIOSampleRate ) g_dSampleRate );
		long lNumInputChannels, lNumOutputChannels;
		ITAsioGetChannels( &lNumInputChannels, &lNumOutputChannels );
		ITAsioCreateBuffers( 0, 2, lBuffersize );
		ITAsioSetPlaybackDatasource( &oProbe );
		ITAsioStart();

	}
	else
	{
		ITAPA.Initialize();
		ITAPA.SetPlaybackDatasource( &oProbe );
		ITAPA.Open();
		ITAPA.Start();
	}

	vstr::out() << "[ NetAudioTestClient ] Waiting 1 second (net audio stream not connected and playing back zeros)" << endl;
	VistaTimeUtils::Sleep( int( 1.0f * 1.0e3 ) );

	vstr::out() << "[ NetAudioTestClient ] Will now connect to net audio server '" << g_sServerName << "' on port " << g_iServerPort << endl;

	if( !oNetAudioStream.Connect( g_sServerName, g_iServerPort, g_bUseUDP ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to net audio server" );
	vstr::out() << "[ NetAudioTestClient ] Connected." << endl;

	// Playback
	float fSeconds = float( g_dDuration );
	vstr::out() << "[ NetAudioTestClient ] Playback started, waiting " << fSeconds << " seconds" << endl;
	VistaTimeUtils::Sleep( int( fSeconds * 1.0e3 ) ); // blocking
	vstr::out() << "[ NetAudioTestClient ] Done." << endl;

	oNetAudioStream.Disconnect();

	vstr::out() << "[ NetAudioTestClient ] Will now disconnect from net audio server '" << g_sServerName << "' and port " << g_iServerPort << endl;
	vstr::out() << "[ NetAudioTestClient ] Closing in 1 second (net audio stream not connected and playing back zeros)" << endl;
	VistaTimeUtils::Sleep( int( 1.0f * 1.0e3 ) );

	if( g_bUseASIO )
	{
		ITAsioStop();
		ITAsioDisposeBuffers();
		ITAsioFinalizeDriver();
		ITAsioFinalizeLibrary();
	}
	else
	{
		ITAPA.Stop();
		ITAPA.Close();
		ITAPA.Finalize();
	}

	delete pServer;

};

int main( int, char** )
{
	try
	{
		run_test();
	}
	catch( ITAException& ie )
	{
		vstr::err() << ie << endl;
		return 255;
	}
	catch( VistaExceptionBase& ve )
	{
		vstr::err() << ve.GetBacktraceString() << endl;
		return 255;
	}

	return 0;

}
