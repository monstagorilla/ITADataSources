#include <iostream>
#include <string>

#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>
<<<<<<< HEAD
#include <ITAFileDataSource.h>
=======
#include <ITAFileDatasource.h>
#include <VistaBase\VistaTimeUtils.h>
>>>>>>> 8eb8596629ad0642aaaec5a68bf1554216289863

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 6)
	{
		fprintf(stderr, "Fehler: Syntax = ServerName ServerPort SampleRate BufferSize Channel!\n");
	}

	string sServerName = argv[1];
	unsigned int iServerPort = atoi(argv[2]);
	double dSampleRate = strtod(argv[3], NULL);
	int iBlockLength = atoi(argv[4]);
	int iChannels = atoi(argv[5]);

	ITAFileDatasource oFile( "gershwin-mono.wav", iBlockLength );
	oFile.SetIsLooping( true );
	ITAStreamMultiplier1N oMuliplier( &oFile, iChannels );
	CITANetAudioStreamingServer oStreamingServer;
	oStreamingServer.SetInputStream( &oMuliplier );

	cout << "Starting net audio server and waiting for connections on '" << sServerName << "' on port " << iServerPort << endl;
	oStreamingServer.Start( sServerName, iServerPort );

	while (!oStreamingServer.IsClientConnected())
	{
		VistaTimeUtils::Sleep(100);
	}
	while (oStreamingServer.IsClientConnected())
	{
		VistaTimeUtils::Sleep(100);
	}

	VistaTimeUtils::Sleep(1000);
	return 0;
}
