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

//static string g_sServerName = "137.226.61.163";
static string g_sServerName = "137.226.61.85";
static int g_iServerPort = 12480;
static double g_dSampleRate = 44100;
static int g_iBufferSize = 32;
static int g_iChannels = 10;

int main( int , char** )
{
	std::cout << "BufferSize: " << endl;
	cin >> g_iBufferSize;
	cout << "ChannelAnzahl: " << endl;
	cin >> g_iChannels;


	CITANetAudioStream oNetAudioStream( g_iChannels, g_dSampleRate, g_iBufferSize, 1 * g_iBufferSize );
	
	ITAStreamPatchbay oPatchbay( g_dSampleRate, g_iBufferSize );
	oPatchbay.AddInput( &oNetAudioStream );
	int iOutputID = oPatchbay.AddOutput( 2 );

	int N = int( oNetAudioStream.GetNumberOfChannels( ) );
	for ( int i = 0; i < N; i++ )
		oPatchbay.ConnectChannels( 0, i, 0, i % 2, 1 / double( N ) );

	ITAStreamProbe oProbe( oPatchbay.GetOutputDatasource( iOutputID ), "ITANetAudioTest.stream.wav" );


	ITAsioInitializeLibrary();

	try {

		cout << "Will now connect to '" << g_sServerName << "' on port " << g_iServerPort << endl;

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


		if (ITAsioSetSampleRate((ASIOSampleRate)g_dSampleRate) != ASE_OK) {
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

		if (!oNetAudioStream.Connect(g_sServerName, g_iServerPort))
			ITA_EXCEPT1(INVALID_PARAMETER, "Could not connect to server");
		VistaTimeUtils::Sleep(2 * 1000);
		printf("Wiedergabe gestartet ...\n");
		VistaTimeUtils::Sleep(20 * 1000);

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
