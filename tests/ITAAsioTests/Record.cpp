#include <ITAException.h>
#include <ITAAsioInterface.h>
#include <ITADataSourceUtils.h>
#include <stdio.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Syntax: play TREIBERNUMMER AUSGABEDATEI\n");
		return 255;
	}

	ITAsioInitializeLibrary();

	long lDriver = atoi(argv[1]);
	
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

		ASIOSampleRate samplerate;
		ITAsioGetSampleRate(&samplerate);

		if (ITAsioCreateBuffers(2, 0, lBuffersize) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n");

			return 255;
		}

		if (ITAsioStart() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

			return 255;
		}

		printf("Aufnahme gestartet ...\n");
		ITADatasource* s = ITAsioGetRecordDatasource();
		// TODO: Nachdem bWaitForData entfernt wurde geht das so nicht mehr.
		// [fwe] Jetzt muss man den Stream seitens der Ausgabe triggern.
		WriteFromDatasourceToFile(s, argv[2], 5*samplerate, 1.0, true, true);
		printf("Aufnahme beendet!\n");

		if (ITAsioStop() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");

			return 255;
		}

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
	} catch (ITAException& e) {
		ITAsioFinalizeLibrary();
		cerr << e << endl;

		return 255;
	}
	
	ITAsioFinalizeLibrary();
	
	return 0;
}