/*
   +------------------------------------------------------------+
   |                                                            |
   |  Loopback2.cpp                                             |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Dieses Programm koppelt Eingänge        |
   |                    auf Ausgänge zurück und schleift        |
   |                    die Audiodaten über eine Instanz        |
   |                    der Klasse ITADatasourceRealization     |
   |                    durch                                   |
   |                                                            |
   |  Motivation:       Probleme mit ASIO-Emulatoren und        |
   |                    ITADatasourceRealization bei manchen    |
   |                    Puffergrößen                            |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  14.2.2005                               |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |                
   +------------------------------------------------------------+
*/

#include <ITAsioInterface.h>
#include <ITADatasourceRealization.h>
#include <stdio.h>

class MySource : public ITADatasourceRealization {
public:
	MySource(ITADatasource* pSource, long lChannels, long lBuffersize)
		: ITADatasourceRealization((unsigned int) lChannels,
		                           (unsigned int) lBuffersize) {
		this->pSource = pSource;
		bFirst = true;
	}
	
	~MySource() {
		printf("MySource::~MySource\n");
		fflush(stdout);
	}

    const float* GetBlockPointer(unsigned int uiChannel, bool bWaitForData=true) {
		if (bFirst) {
			for (unsigned int i=0; i<m_uiChannels; i++)
				memcpy(GetWritePointer(i),
				       pSource->GetBlockPointer(i, bWaitForData),
					   m_uiBlocklength * sizeof(float));
			IncrementWritePointer();
			pSource->IncrementBlockPointer();
			bFirst = false;
		}

		return ITADatasourceRealization::GetBlockPointer(uiChannel, bWaitForData);	
	}
	
	void IncrementBlockPointer() {
		ITADatasourceRealization::IncrementBlockPointer();
		bFirst = true;
	}

private:
	ITADatasource* pSource;
	bool bFirst;
};

int main(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Syntax: loopback TREIBERNUMMER KANAELE PUFFERGROESSE\n");
		return 255;
	}

	long lDriver = atoi(argv[1]);
	long lChannels = atoi(argv[2]);
	long lBuffersize = atoi(argv[3]);
	
	if (ITAsioInit(lDriver-1) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioInit schlug fehl!\n");
		return 255;
	}		

	if (ITAsioCreateBuffers(lChannels, lChannels, lBuffersize) != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n");
		return 255;
	}

	MySource gateway(ITAsioGetRecordDatasource(), lChannels, lBuffersize);
	ITAsioSetPlaybackDatasource(&gateway);

	if (ITAsioStart() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");
		return 255;
	}

	printf("ASIO-Streaming gestartet ... Taste drücken zum Beenden!\n");
	getchar();
	printf("ASIO-Streaming beendet!\n");

	if (ITAsioStop() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");
		return 255;
	}

	printf("DisposeBuffers\n");
	if (ITAsioDisposeBuffers() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");
		return 255;
	}

	if (ITAsioExit() != ASE_OK) {
		fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");
		return 255;
	}

	return 0;
}