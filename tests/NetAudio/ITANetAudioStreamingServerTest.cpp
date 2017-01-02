#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITAStreamFunctionGenerator.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44.1e3;
static int g_iBlockLength = 256;

int main( int , char** )
{
	ITAStreamFunctionGenerator oGenerator( 1, g_dSampleRate, g_iBlockLength, ITAStreamFunctionGenerator::SINE, 456.78f, 0.81f, true );

	CITANetAudioStreamingServer oStreamingServer;
	oStreamingServer.SetInputStream( &oGenerator );

	cout << "Starting server and waiting for connections on '" << g_sServerName << "' on port " << g_iServerPort << endl;
	oStreamingServer.Start( g_sServerName, g_iServerPort );

	int iKey;
	cin >> iKey;
	
	return 0;
}
