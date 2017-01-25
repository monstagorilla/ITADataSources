#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAFileDatasource.h>

using namespace std;

static string g_sServerName = "137.226.61.67";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44100;
static int g_iBlockLength = 512;
static int g_iChannels = 500;

int main( int , char** )
{
	//ITAStreamFunctionGenerator oGenerator( 2, g_dSampleRate, g_iBlockLength, ITAStreamFunctionGenerator::SINE, 456.78f, 0.81f, true );
	ITAFileDatasource oFile("gershwin-mono.wav", g_iBlockLength);
	ITAStreamMultiplier1N oMuliplier( &oFile, g_iChannels );
	CITANetAudioStreamingServer oStreamingServer;
	oStreamingServer.SetInputStream( &oMuliplier );

	cout << "Starting server and waiting for connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
	oStreamingServer.Start( g_sServerName, g_iServerPort );

	int iKey;
	std::cin >> iKey;
	
	return 0;
}
