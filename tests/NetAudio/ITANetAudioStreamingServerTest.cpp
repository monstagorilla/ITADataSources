#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAFileDatasource.h>

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 5)
		fprintf(stderr, "Fehler: Syntax = ServerName ServerPort SampleRate BufferSize Channel!\n");

	static string sServerName = argv[1];
	static int iServerPort = (int)argv[2];
	static double dSampleRate = strtod(argv[3], NULL);
	static int iBlockLength = (int)argv[4];
	static int iChannels = (int)argv[5];

	ITAFileDatasource oFile( "gershwin-mono.wav", iBlockLength );
	oFile.SetIsLooping( true );
	ITAStreamMultiplier1N oMuliplier( &oFile, iChannels );
	CITANetAudioStreamingServer oStreamingServer;
	oStreamingServer.SetInputStream( &oMuliplier );

	cout << "Starting net audio server and waiting for connections on '" << sServerName << "' on port " << iServerPort << endl;
	oStreamingServer.Start( sServerName, iServerPort );

	int iKey;
	std::cin >> iKey;

	return 0;
}
