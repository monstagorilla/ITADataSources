#include <iostream>
#include <string>

#include <ITANetAudioSampleServer.h>
#include <ITAStreamFunctionGenerator.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44.1e3;

int main( int , char** )
{
	ITAStreamFunctionGenerator oGenerator();
	CITANetAudioSampleServer oSampleServer();

	oSampleServer.Start( g_iServerPort, g_iServerPort );
	
	return 0;
}
