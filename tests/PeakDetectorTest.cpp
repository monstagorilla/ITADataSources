#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAPeakDetector.h>
#include <ITAStringUtils.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	
	if (argc != 2) {
		cerr << "Syntax: StreamAnalyzerTest INPUTFILE\n" << endl;
		return 255;
	}
	
	ITAFileDatasource* pSource = 0;
	ITAPeakDetector* pDetector = 0;

	try {
		cout << "Input file: \"" << argv[1] << "\"\n" << endl;
		pSource = new ITAFileDatasource(argv[1], 1024);
		pDetector = new ITAPeakDetector(pSource);

		unsigned int n = pSource->GetCapacity() / pSource->GetBlocklength();
		unsigned int c = pSource->GetNumberOfChannels();
		for (unsigned int i=0; i<n; i++) {
			for (unsigned int j=0; j<c; j++) pDetector->GetBlockPointer(j, 0);
			pDetector->IncrementBlockPointer();
		}

		// Analyseinformationen ausgeben
		vector<double> vdPeaksDecibel;
		pDetector->GetPeaksDecibel(vdPeaksDecibel);

		for (unsigned int i=0; i<vdPeaksDecibel.size(); i++)
			cout << "Kanal " << (i+1) << ": " << DecibelToString(vdPeaksDecibel[i]) << endl;
		
		double dOverall;
		unsigned int uiChannel;
		pDetector->GetOverallPeakDecibel(&dOverall, &uiChannel);
		cout << endl << "Insgesamt: " << DecibelToString(dOverall) << " in Kanal " << (uiChannel+1) << endl;

		delete pDetector;
		delete pSource;
	} catch (ITAException& e) {
		delete pDetector;
		delete pSource;
		cerr << "Error: " << e << endl;
	}

	return 0;
}
