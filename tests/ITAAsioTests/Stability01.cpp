/*
   +------------------------------------------------------------+
   |                                                            |
   |  Stability01.cpp                                           |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Stablitätstest 01                       |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  13.04.2011 [stienen]                    |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |                
   +------------------------------------------------------------+
*/
// $Id: Stability01.cpp 1801 2011-04-13 14:07:25Z stienen $

#include <ITAsioInterface.h>
#include <ITAException.h>
#include <stdio.h>
#include <windows.h>

/*
   Testbeschreibung: Zyklus aus Init->CreateBuffers->Start
                     ->Stop->DisposeBuffers->Exit mehrmals durchlaufen.
*/

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Syntax: Stability01 TREIBERNUMMER\n");

		return 255;
	}

	ITAsioInitializeLibrary();

	long lDriver = atoi(argv[1]);
	
	try {
		for (unsigned int i=0; i<5; i++) {
			printf("Durchgang #%d von #%d\n\n", i+1, 5);

			printf("-> ITAsioInit\n");
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

			long lInputChannels, lOutputChannels;
			if (ITAsioGetChannels(&lInputChannels, &lOutputChannels) != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioGetChannels schlug fehl!\n");

				return 255;
			}

			printf("-> ITAsioCreateBuffers\n");
			if (ITAsioCreateBuffers(lInputChannels, lOutputChannels, lBuffersize) != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n");

				return 255;
			}

			printf("-> ITAsioStart\n");
			if (ITAsioStart() != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

				return 255;
			}

			Sleep(2000);

			printf("-> ITAsioStop\n");
			if (ITAsioStop() != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");
				return 255;

			}

			printf("-> ITAsioDisposeBuffers");
			if (ITAsioDisposeBuffers() != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");

				return 255;
			}

			printf("-> ITAsioExit\n\n");
			if (ITAsioFinalizeDriver() != ASE_OK) {
				ITAsioFinalizeLibrary();
				fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");

				return 255;
			}
		}
	} catch (ITAException e) {
		ITAsioFinalizeLibrary();
		fprintf(stderr, "Fehler: %s\n", e.toString().c_str());

		return 255;
	}

	ITAsioFinalizeLibrary();

	printf("Test beendet\n");

	return 0;
}