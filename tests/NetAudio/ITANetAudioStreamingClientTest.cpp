#include <iostream>
#include <string>

#include <ITANetAudioStream.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAStreamProbe.h>
#include <ITAStreamPatchbay.h>
#include <ITAAsioInterface.h>
#include <VistaBase\VistaTimeUtils.h>

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 6)
		fprintf(stderr, "Fehler: Syntax = ServerName ServerPort SampleRate BufferSize Channel RingBufferSize!\n");

	static string sServerName = argv[1];
	static int iServerPort = (int)argv[2];
	static double dSampleRate = strtod(argv[3], NULL);
	static int iBlockLength = (int)argv[4];
	static int iChannels = (int)argv[5];
	static int iBufferSize = (int)argv[6];



	CITANetAudioStream oNetAudioStream(iChannels, dSampleRate, iBlockLength, 1 * iBufferSize);
	
	ITAStreamPatchbay oPatchbay(dSampleRate, iBlockLength);
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels( ) );
	for ( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );

	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.stream.wav" );


	ITAsioInitializeLibrary();

	try {

		cout << "Will now connect to '" << sServerName << "' on port " << iServerPort << endl;

		if (ITAsioInitializeDriver("ASIO MADIface USB") != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioInit schlug fehl!\n");

			return 255;
		}

		long lBuffersize, lDummy;
		if (ITAsioGetBufferSize(&lDummy, &lDummy, &lBuffersize, &lDummy) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioGetBufferSize schlug fehl!\n");

			return 255;
		}


		if (ITAsioSetSampleRate((ASIOSampleRate)dSampleRate) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioSetSamplerate schlug fehl!\n");

			return 255;
		}

		long lNumInputChannels, lNumOutputChannels;
		ASIOError ae;
		if ((ae = ITAsioGetChannels(&lNumInputChannels, &lNumOutputChannels)) != ASE_OK)
		{
			cerr << "Error in ITAsioGetChannels, ASIO error " << ae << " encountered" << endl;
			ITAsioFinalizeLibrary();
			return 255;
		}

		if ((ae = ITAsioCreateBuffers(0, 2, lBuffersize)) != ASE_OK)
		{
			cerr << "Error in ITAsioCreateBuffers, ASIO error " << ae << " encountered" << endl;
			ITAsioFinalizeLibrary();
			return 255;
		}

		
		ITAsioSetPlaybackDatasource(&oProbe);

		if (ITAsioStart() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

			return 255;
		}

		if (!oNetAudioStream.Connect(sServerName, iServerPort))
			ITA_EXCEPT1(INVALID_PARAMETER, "Could not connect to server");
		printf("Wiedergabe gestartet ...\n");
		VistaTimeUtils::Sleep(10 * 1000);

		if (ITAsioStop() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");

			return 255;
		}

		printf("Wiedergabe beendet!\n");

		if (ITAsioDisposeBuffers() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");

			return 255;
		}

		if (ITAsioFinalizeDriver() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");

			return 255;
		}
	}
	catch (ITAException e) {
		ITAsioFinalizeLibrary();
		cerr << e << endl;

		return 255;
	}

	ITAsioFinalizeLibrary();

	
	return 0;
}
