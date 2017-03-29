#include <iostream>
#include <string>
#include <sstream>

#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchBay.h>
#include <ITAAsioInterface.h>
#include <VistaBase/VistaTimeUtils.h>

using namespace std;

string g_sServerName = "localhost";
int g_iServerPort = 12480;
double g_dSampleRate = 44100.0;
int g_iBlockLength = 512;
int g_iChannels = 2;
int g_iTargetLatencySamples = 2 * g_iBlockLength; // 1.4512ms
int g_iRingBufferSize = 2 * g_iTargetLatencySamples;
double g_dPlaybackDuration = 10 ; // seconds

int main( int argc, char* argv[] )
{
	if( argc >= 8 )
	{
		g_sServerName = argv[ 1 ];

		if( argc >= 3 )
		{
			g_iServerPort = atoi( argv[ 2 ] );
			g_dSampleRate = strtod( argv[ 3 ], NULL );
			g_iBlockLength = atoi( argv[ 4 ] );
			g_iChannels = atoi(argv[5]);
			g_iTargetLatencySamples = atoi(argv[6]);
			g_iRingBufferSize = atoi(argv[7]);
		}

		if( argc >= 9 )
			g_dPlaybackDuration = strtod( argv[ 8 ], NULL );;
	}
	else
	{
		cout << "Syntax: ServerName ServerPort SampleRate BufferSize Channel TargetLatencySamples RingBufferSize" << endl;
		cout << "Using default values ..." << endl;
	}

	cout << "Number of NetAudio channels: " << g_iChannels << endl;

	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBlockLength, g_iRingBufferSize );

	stringstream ss;
	ss << "ITANetAudioStreamingClientTest";
	ss << "_C" << g_iChannels;
	ss << "_B" << g_iBlockLength;
	ss << "_TL" << g_iTargetLatencySamples;
	ss << "_RB" << g_iRingBufferSize;
	oNetAudioStream.SetNetAudioStreamingLoggerBaseName( ss.str() );

	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBlockLength );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels() );
	for( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );

	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.netstream.wav" );


	cout << "Will attempt to connect to '" << g_sServerName << "' on port " << g_iServerPort << endl;

	ITAsioInitializeLibrary();
	ITAsioInitializeDriver( "ASIO4ALL v2" );

	long lBuffersize, lDummy;
	ITAsioGetBufferSize( &lDummy, &lDummy, &lBuffersize, &lDummy );
	ITAsioSetSampleRate( ( ASIOSampleRate ) g_dSampleRate );
	long lNumInputChannels, lNumOutputChannels;
	ITAsioGetChannels( &lNumInputChannels, &lNumOutputChannels );
	ITAsioCreateBuffers( 0, 2, lBuffersize );
	ITAsioSetPlaybackDatasource( &oProbe );
	
	ITAsioStart();
	cout << "ASIO streaming started." << endl;

	try
	{
		cout << "Connecting to NetAudio server ..." << endl;
		if( !oNetAudioStream.Connect( g_sServerName, g_iServerPort ) )
			ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to server" );
		cout << "Connection established." << endl;

		cout << "Will now stream for " << g_dPlaybackDuration << " seconds ..." << endl;
		VistaTimeUtils::Sleep( int( g_dPlaybackDuration ) * 1000 );
	}
	catch( ITAException& e )
	{
		cerr << e << endl;
	}

	cout << "Stopping ASIO stream and finalizing." << endl;
	ITAsioStop();

	ITAsioDisposeBuffers();
	ITAsioFinalizeDriver();
	ITAsioFinalizeLibrary();
	return 0;
}
