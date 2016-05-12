/*
   +------------------------------------------------------------+
   |                                                            |
   |  Measure.cpp                                               |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Dieses Programm spielt eine Audiodatei  |
   |                    ab und nimmt dabei gleichzeitig vom     |
   |                    Gerät auf                               |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  13.04.2011 [stienen]                    |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |                
   +------------------------------------------------------------+
*/
// $Id: Measure.cpp 1801 2011-04-13 14:07:25Z stienen $

#include <ITAsioInterface.h>
#include <ITADatasourceUtils.h>
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Syntax: measure TREIBERNUMMER EINGABEDATEI AUSGABEDATEI\n");
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

		printf("Buffersize = %d\n", lBuffersize);

		ITAFileDatasource source(argv[2], (unsigned int) lBuffersize, false);

		if (ITAsioSetSampleRate((ASIOSampleRate) source.GetSamplerate()) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioSetSamplerate schlug fehl!\n");

			return 255;
		}

		if (ITAsioCreateBuffers((long) source.GetNumberOfChannels(), (long) source.GetNumberOfChannels(), lBuffersize) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n");

			return 255;
		}

		ITAsioSetPlaybackDatasource(&source);

		if (ITAsioStart() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

			return 255;
		}

		printf("ASIO-Streaming gestartet ...\n");
		WriteFromDatasourceToFile(ITAsioGetRecordDatasource(), argv[3], source.GetCapacity(), 1.0, false, false);
		printf("ASIO-Streaming beendet!\n");

		if (ITAsioStop() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");

			return 255;
		}

		if (ITAsioDisposeBuffers() != ASE_OK) {
			fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");
			return 255;
		}

		if (ITAsioFinalizeDriver() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");

			return 255;
		}
	} catch (ITAException e) {
		ITAsioFinalizeLibrary();
		fprintf(stderr, "Fehler: %s\n", e.toString().c_str());

		return 255;
	}
	
	ITAsioFinalizeLibrary();
	
	return 0;
}