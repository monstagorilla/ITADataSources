/*
   +-------------------------------------------------------+
   |                                                       |
   |  Patchbay1Test.h                                      |
   |  Test der Klasse ITAStreamPatchbayType1               |
   |  Teil der ITADatasources-Testsuite                    |
   |                                                       |
   |  Autoren: Frank Wefers                                |
   |                                                       |
   |  (c) Copyright Institut für technische Akustik (ITA)  |
   |      Aachen university of technology (RWTH), 2008     |
   |                                                       |
   +-------------------------------------------------------+
                                                             */
// $Id: Patchbay1Test.cpp,v 1.1 2008-09-19 15:28:43 fwefers Exp $

#include <ITAAudiofileWriter.h>
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAFileDatasink.h>
#include <ITABufferDatasink.h>
#include <ITAStreamFilter.h>
#include <ITAStreamPatchbayType1.h>
#include <ITAStringUtils.h>
#include <iostream>

using namespace std;

/* Variante mit FileDatasink 
int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		cerr << "Syntax: Patchbay1Test INPUTFILE OUTPUFILE\n" << endl;
		return 255;
	}
	
	ITAFileDatasource* pSource = 0;
	ITAFileDatasink* pSink = 0;
	ITAStreamPatchbayType1* pPatchbay = 0;

	try {
		cout << "Input file: \"" << argv[1] << "\"\n" << endl;
		pSource = new ITAFileDatasource(argv[1], 1024);
		
		pPatchbay = new ITAStreamPatchbayType1(pSource, 4);
		pPatchbay->WireOutputChannel(3, 1);
		pPatchbay->WireOutputChannel(0, 1);
		pPatchbay->WireOutputChannel(1, 0);


		pSink = new ITAFileDatasink("out.wav", pPatchbay);
	
		pSink->Transfer(pSource->GetCapacity());
		//pSink->Transfer(1000);

		delete pSource;
		delete pPatchbay;
		delete pSink;
		
	} catch (ITAException& e) {

		delete pSource;
		delete pPatchbay;
		delete pSink;
		
		cerr << "Error: " << e.toString() << endl;
	}

	return 0;
}
*/

int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		cerr << "Syntax: Patchbay1Test INPUTFILE OUTPUFILE\n" << endl;
		return 255;
	}
	
	ITAFileDatasource* pSource = 0;
	ITABufferDatasink* pSink = 0;
	ITAStreamPatchbayType1* pPatchbay = 0;

	try {
		cout << "Input file: \"" << argv[1] << "\"\n" << endl;
		pSource = new ITAFileDatasource(argv[1], 1024);
		
		pPatchbay = new ITAStreamPatchbayType1(pSource, 4);
		pPatchbay->WireOutputChannel(3, 1);
		pPatchbay->WireOutputChannel(0, 1);
		pPatchbay->WireOutputChannel(1, 0);


		pSink = new ITABufferDatasink(pPatchbay, pSource->GetCapacity());
	
		pSink->Transfer(pSource->GetCapacity());
		//pSink->Transfer(1000);

		// Daten runterschreiben
		ITAAudiofileProperties props;
		props.dSamplerate = pPatchbay->GetSamplerate();
		props.eDomain = ITA_TIME_DOMAIN;
		props.eQuantization = ITA_INT16;
		props.uiChannels = pPatchbay->GetNumberOfChannels();
		props.uiLength = pSink->GetBuffersize();
		writeAudiofile("out.wav", props, pSink->GetBuffers());

		delete pSource;
		delete pPatchbay;
		delete pSink;
		
	} catch (ITAException& e) {

		delete pSource;
		delete pPatchbay;
		delete pSink;
		
		cerr << "Error: " << e.toString() << endl;
	}

	return 0;
}