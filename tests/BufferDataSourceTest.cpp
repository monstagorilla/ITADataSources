#include <ITAException.h>
#include <ITABufferDataiSource.h>
#include <ITADataSourceUtils.h>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {

	ITABufferDatasource* pSource = 0;
	const unsigned int uiBuffersize = 1000;
	float* pfRamp = new float[uiBuffersize];
	for (unsigned int i=0; i<uiBuffersize; i++)
		pfRamp[i] = 2*(((float) i) / ((float) uiBuffersize-1)) - 1;
	float* pfInvRamp = new float[uiBuffersize];
	for (unsigned int i=0; i<uiBuffersize; i++)
		pfInvRamp[i] = 1 - 2*(((float) i) / ((float) uiBuffersize-1));
	std::vector<float*> v;
	v.push_back(pfRamp);
	v.push_back(pfInvRamp);

	try {
		pSource = new ITABufferDatasource(v, uiBuffersize, 44100, 128);
		
		printf("Blocklength: %d\n", pSource->GetBlocklength());
		printf("Capacity: %d\n", pSource->GetCapacity());
		printf("Cursor: %d\n", pSource->GetCursor());
		printf("AbsCursor: %d\n", pSource->GetAbsoluteCursor());
		printf("ROI: [%d, %d)\n", pSource->GetROIStart(), pSource->GetROIEnd());
		printf("ROI-Length: %d\n", pSource->GetROILength());

		// Kompletten Inhalt in eine Datei schreiben
		WriteFromDatasourceToFile(pSource, "Complete.wav", pSource->GetCapacity());

		// Rückspulen testen
		pSource->Rewind();
		WriteFromDatasourceToFile(pSource, "CompleteRewind.wav", pSource->GetCapacity());

		// Wiederholungsmodus testen
		printf("\n\nLoop!\n\n\n");
		pSource->SetLoopMode(true);
		//pSource->Rewind();
		WriteFromDatasourceToFile(pSource, "Loop.wav", uiBuffersize*4);

		// Ausschnitt testen
		printf("\n\nAusschnitt!\n\n\n");

		pSource->SetLoopMode(false);
		pSource->Rewind();
		pSource->SetROI(300, 600);

		printf("Capacity: %d\n", pSource->GetCapacity());
		printf("Cursor: %d\n", pSource->GetCursor());
		printf("AbsCursor: %d\n", pSource->GetAbsoluteCursor());
		printf("ROI: [%d, %d)\n", pSource->GetROIStart(), pSource->GetROIEnd());
		printf("ROI-Length: %d\n", pSource->GetROILength());

		//pSource->Rewind();
		WriteFromDatasourceToFile(pSource, "Section.wav", pSource->GetROILength());

		printf("Capacity: %d\n", pSource->GetCapacity());
		printf("Cursor: %d\n", pSource->GetCursor());
		printf("AbsCursor: %d\n", pSource->GetAbsoluteCursor());
		printf("ROI: [%d, %d)\n", pSource->GetROIStart(), pSource->GetROIEnd());
		printf("ROI-Length: %d\n", pSource->GetROILength());

		// Wiederholten Ausschnitt testen
		printf("\n\nLoop-Ausschnitt!\n\n\n");
		pSource->SetLoopMode(true);
		pSource->Rewind();
		WriteFromDatasourceToFile(pSource, "SectionLoop.wav", pSource->GetROILength()*3);

		// Kleineren wiederholten Ausschnitt testen
		pSource->SetROI(256, 384);
		pSource->Rewind();
		WriteFromDatasourceToFile(pSource, "SectionLoopShort.wav", pSource->GetROILength()*3);

		// Leere Sektion
		pSource->SetROI(800, 800);
		WriteFromDatasourceToFile(pSource, "EmptyLoop.wav", pSource->GetROILength());

		delete pSource;
	} catch (ITAException& e) {
		delete pSource;

		cerr << e << std::endl;
	}

	delete pfRamp;
	delete pfInvRamp;

	return 0;
}
