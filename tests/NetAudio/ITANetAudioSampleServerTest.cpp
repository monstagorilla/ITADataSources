#include <iostream>
#include <string>

#include <ITANetAudioSampleServer.h>
#include <ITAStreamFunctionGenerator.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44.1e3;
static int g_iBlockLength = 265;

int main( int , char** )
{
	ITAStreamFunctionGenerator oGenerator( 1, g_dSampleRate, g_iBlockLength, ITAStreamFunctionGenerator::SINE, 456.78f, 0.81f, true );

	CITANetAudioSampleServer oSampleServer;
	oSampleServer.SetInputStream( &oGenerator );

	cout << "Starting server and waiting for connections on '" << oSampleServer.GetNetworkAddress() << "' on port " << oSampleServer.GetNetworkPort() << endl;
	oSampleServer.Start( g_sServerName, g_iServerPort );
	
	return 0;
}
