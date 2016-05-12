/*
   +------------------------------------------------------------+
   |                                                            |
   |  Stability02.cpp                                           |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Stablitätstest 02                       |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  13.04.2011 [stienen]                    |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |                
   +------------------------------------------------------------+
*/
// $Id: Stability02.cpp 1801 2011-04-13 14:07:25Z stienen $


#include <ITAsioInterface.h>
#include <stdio.h>
#include <windows.h>

/*
   Testbeschreibung: CreateBuffers einmal ohne Eingabekanäle,
                     und einmal ohne Ausgabekanäle durchführen
					 und anschließend das jeweils Streaming laufen lassen.
*/

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Syntax: Stability02 TREIBERNUMMER\n");
		return 255;
	}

	long lDriver = atoi(argv[1]);
	
	if (ITAsioInit(lDriver-1) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioInit schlug fehl!\n");
		return 255;
	}		

	long lBuffersize, lDummy;
	if (ITAsioGetBufferSize(&lDummy, &lDummy, &lBuffersize, &lDummy) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioGetBufferSize schlug fehl!\n");
		return 255;
	}

	printf("CreateBuffers(2, 0, ...)\n\n");
	if (ITAsioCreateBuffers(2, 0, lBuffersize) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioCreateBuffers(2,0) schlug fehl!\n");
		return 255;
	}

	if (ITAsioStart() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");
		return 255;
	}

	printf("Streaming gestartet ... \n");
	Sleep(2000);
	printf("Streaming beendet!\n\n");

	if (ITAsioStop() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");
		return 255;
	}

	if (ITAsioDisposeBuffers() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");
		return 255;
	}

	printf("CreateBuffers(0, 2, ...)\n\n");
	if (ITAsioCreateBuffers(0, 2, lBuffersize) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioCreateBuffers(0,2) schlug fehl!\n");
		return 255;
	}

	if (ITAsioStart() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");
		return 255;
	}

	printf("Streaming gestartet ... \n");
	Sleep(2000);
	printf("Streaming beendet!\n\n");

	if (ITAsioStop() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");
		return 255;
	}

	if (ITAsioDisposeBuffers() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");
		return 255;
	}

	if (ITAsioExit() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");
		return 255;
	}

	printf("Test beendet\n");

	return 0;
}