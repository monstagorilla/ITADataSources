#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITANetAudioStreamingServer.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <VistaBase/VistaTimeUtils.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

string g_sServerName                 = "137.226.61.85";
int g_iServerPort                    = 12480;
double g_dSampleRate                 = 44100.0;
int g_iBlockLength                   = 128;
int g_iChannels                      = 10;
int g_iTargetLatencySamples          = 10 * g_iBlockLength; // 1.4512ms
int g_iRingBufferSize                = 2 * g_iTargetLatencySamples;
int g_iSendingBlockLength            = 8;
double g_dClientStatusMessageTimeout = 0.001; // seconds
double g_dEstimatiedCorrFactor       = 1;
string g_sFileName                   = "gershwin-mono.wav";
bool g_bDebuggingEnabled             = true;

int main( int argc, char** argv )
{
	if( argc >= 9 )
	{
		g_sServerName = argv[1];

		if( argc >= 3 )
		{
			g_iServerPort           = atoi( argv[2] );
			g_dSampleRate           = strtod( argv[3], NULL );
			g_iBlockLength          = atoi( argv[4] );
			g_iChannels             = atoi( argv[5] );
			g_iTargetLatencySamples = atoi( argv[6] );
			g_iRingBufferSize       = atoi( argv[7] );
			g_iSendingBlockLength   = atoi( argv[8] );
		}
		if( argc >= 9 )
			g_dEstimatiedCorrFactor = atoi( argv[9] );
	}
	else
	{
		cout << "Syntax: ServerName ServerPort SampleRate BufferSize Channel TargetLatencySamples RingBufferSize SnedingBlockLength EstimatiedCorrFactor" << endl;
		cout << "Using default values ..." << endl;
	}

	ITADatasource* pSource = NULL;
	try
	{
		pSource = new ITAFileDatasource( g_sFileName, g_iBlockLength );
		static_cast<ITAFileDatasource*>( pSource )->SetIsLooping( true );
		cout << "Found file " << g_sFileName << ", will use it for playback." << endl;
	}
	catch( ITAException& )
	{
		cout << "Could not find file " << g_sFileName << ", will use SINE signal instead." << endl;
		pSource = new ITAStreamFunctionGenerator( 1, g_dSampleRate, g_iBlockLength, ITAStreamFunctionGenerator::SINE, 250.0f, 0.7171f, true );
	}

	ITAStreamMultiplier1N oMuliplier( pSource, g_iChannels );
	CITANetAudioStreamingServer oStreamingServer;

	stringstream ss;
	ss << "ITANetAudioStreamingServerTest";
	ss << "_C" << g_iChannels;
	ss << "_B" << g_iBlockLength;
	ss << "_TL" << g_iTargetLatencySamples;
	ss << "_RB" << g_iRingBufferSize;
	ss << "_SB" << g_iSendingBlockLength;
	ss << "_EC" << g_dEstimatiedCorrFactor;
	oStreamingServer.SetServerLogBaseName( ss.str( ) );
	oStreamingServer.SetDebuggingEnabled( g_bDebuggingEnabled );
	oStreamingServer.SetEstimatedCorrFactor( g_dEstimatiedCorrFactor );
	oStreamingServer.SetInputStream( &oMuliplier );
	oStreamingServer.SetTargetLatencySamples( g_iTargetLatencySamples );
	oStreamingServer.SetSendingBlockLength( g_iSendingBlockLength );

	cout << "Starting net audio server and waiting for connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
	if( oStreamingServer.Start( g_sServerName, g_iServerPort, g_dClientStatusMessageTimeout ) )
		cout << "Client connected, sending samples." << endl;
	else
	{
		cerr << "Connection failed or streaming refused, aborting." << endl;
		return 255;
	}

	while( !oStreamingServer.IsClientConnected( ) )
		VistaTimeUtils::Sleep( 100 );

	while( oStreamingServer.IsClientConnected( ) )
		VistaTimeUtils::Sleep( 100 );

	VistaTimeUtils::Sleep( 2000 );

	delete pSource;

	return 0;
}
