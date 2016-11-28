#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITADataSourceUtils.h>
#include <ITAStreamFilterType1.h>
#include <ITAStringUtils.h>
#include <iostream>
#include <math.h>

using namespace std;

class DSJoin : public ITADatasource {
public:
	DSJoin(ITADatasource* pDS1, ITADatasource* pDS2)
		: ITADatasource(pDS1->GetBlocklength(), 2*pDS1->GetNumberOfChannels(), pDS1->GetSamplerate()), m_pDS1(pDS1), m_pDS2(pDS2) {}

	const float* GetBlockPointer(unsigned int uiChannel, bool bWaitForData=true) {
		if (uiChannel < m_pDS1->GetNumberOfChannels())
			return m_pDS1->GetBlockPointer(uiChannel, bWaitForData);
		else
			return m_pDS2->GetBlockPointer(uiChannel - m_pDS1->GetNumberOfChannels(), bWaitForData);
	}
	
	void IncrementBlockPointer() {
		m_pDS1->IncrementBlockPointer();
		m_pDS2->IncrementBlockPointer();
	}

private:
	ITADatasource* m_pDS1;
	ITADatasource* m_pDS2;
};


class MyFilter : public ITAStreamFilterType1 {
public:
	void ProcessChannel(unsigned int uiChannel, const float* pfInputData, float* pfOutputData) {
		for (int i=0; i<m_uiBlocklength; i++) {
			//pfOutputData[i] = -pfInputData[i];
		
			/*
			if (pfInputData[i] == 0)
				pfOutputData[i] = 0;
			if (pfInputData[i] < 0)
				pfOutputData[i] = -sqrt(-pfInputData[i]);
			else
				pfOutputData[i] = sqrt(pfInputData[i]);
			*/

			int k=0;
			float s=0;
			for (int j=-6; j<6; j++)
				if ((i+j >= 0) && (i+j < m_uiBlocklength)) {
					s += pfInputData[i+j];
					k++;
				}
			pfOutputData[i] = s/k;
		}
	}
};

int main(int argc, char* argv[]) {
/*	
	if (argc != 3) {
		cerr << "Syntax: FilterTest INPUTFILE OUTPUFILE\n" << endl;
		return 255;
	}
	
	ITAFileDatasource* pSource = 0;
	MyFilter filter;

	try {
		cout << "Input file: \"" << argv[1] << "\"\n" << endl;
		pSource = new ITAFileDatasource(argv[1], 1024);
		
		filter.SetInputDatasource(pSource);
		WriteFromDatasourceToFile(&filter, argv[2], pSource->GetCapacity(), 1.0, false);

		delete pSource;
	} catch (ITAException& e) {
		delete pSource;
		cerr << "Error: " << e.toString() << endl;
	}
*/

	if (argc != 4) {
		cerr << "Syntax: FilterTest INPUTFILE INPUT OUTPUFILE\n" << endl;
		return 255;
	}
	
	ITAFileDatasource* pSource1 = 0;
	ITAFileDatasource* pSource2 = 0;
	DSJoin* pJoin = 0;

	try {
		cout << "Input file 1: \"" << argv[1] << "\"\n" << endl;
		pSource1 = new ITAFileDatasource(argv[1], 1024);

		cout << "Input file 2: \"" << argv[2] << "\"\n" << endl;
		pSource2 = new ITAFileDatasource(argv[2], 1024);
		pSource2->SetLoopMode(true);
		
		pJoin = new DSJoin(pSource1, pSource2);

		WriteFromDatasourceToFile(pJoin, argv[3], max(pSource1->GetCapacity(), pSource2->GetCapacity()), 1.0, false);

		delete pSource1;
		delete pSource2;
		delete pJoin;
	} catch (ITAException& e) {
		delete pSource1;
		delete pSource2;
		delete pJoin;
		cerr << "Error: " << e.toString() << endl;
	}
	return 0;
}