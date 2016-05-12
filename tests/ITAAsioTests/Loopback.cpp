#include <ITAStreamAmplifier.h>
#include <ITAStreamProbe.h>
#include <ITAAsioInterface.h>
#include <ITAException.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Syntax: loopback TREIBERNUMMER\n");

		return 255;
	}

	ITAsioInitializeLibrary();

	long lDriver = atoi(argv[1]);

	ITAStreamAmplifier* pAmp = NULL;
	ITAStreamProbe* pProbe = NULL;
	ITADatasource* pTail;
	 
	try {
		if (ITAsioInitializeDriver(lDriver-1) != ASE_OK) {
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

		long lChannels = 2;

		if (ITAsioCreateBuffers(lChannels, lChannels, lBuffersize) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n");

			return 255;
		}

		ITADatasource* pSource = ITAsioGetRecordDatasource();
		pTail = pSource;
	
		/*
		pAmp = new ITAStreamAmplifier(pSource);
		pTail = pAmp;

		if (true) {
			pProbe = new ITAStreamProbe(pTail, "Loopback.wav");
			pTail = pProbe;
		}
		*/
	
		ITAsioSetPlaybackDatasource(pTail);

		if (ITAsioStart() != ASE_OK) {
			ITAsioFinalizeLibrary();
			delete pAmp;
			pAmp = NULL;

			delete pProbe;
			pProbe = NULL;

			fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

			return 255;
		}

		printf("ASIO-Streaming gestartet ... Taste druecken zum Beenden!\n");
		getchar(); // <-- crasht

		printf("ASIO-Streaming gestartet ... 3 Sekunden warten!\n");
		//Sleep(100); // <-- crasht

		if (ITAsioStop() != ASE_OK) {
			ITAsioFinalizeLibrary();
			delete pAmp;
			pAmp = NULL;

			delete pProbe;
			pProbe = NULL;

			fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");

			return 255;
		}
		printf("ASIO-Streaming beendet!\n");

		if (ITAsioDisposeBuffers() != ASE_OK) {
			ITAsioFinalizeLibrary();
			delete pAmp;
			pAmp = NULL;
			delete pProbe;
			pProbe = NULL;
			fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");

			return 255;
		}
		
		ITAsioFinalizeLibrary();
		delete pAmp;
		pAmp = NULL;
		delete pProbe;
		pProbe = NULL;

	} catch (ITAException e) {
		ITAsioFinalizeLibrary();
		delete pAmp;
		pAmp = NULL;
		delete pProbe;
		pProbe = NULL;
		cerr << e << endl;

		return 255;
	}

	ITAsioFinalizeLibrary();
	delete pAmp;
	pAmp = NULL;
	delete pProbe;
	pProbe = NULL;

	return 0;
}