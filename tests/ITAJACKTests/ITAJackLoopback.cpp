// $Id: ITAPortaudioInterfaceRecorder.cpp 2333 2012-03-05 14:21:39Z stienen $

#include <iostream>
#include <string>
#include <ITADataSourceUtils.h>
#include <ITAJACKInterface.h>
#include <ITADataSourceRealization.h>

#include <ITAFileDatasource.h>
#include <ITAStreamFunctionGenerator.h>

static double dSampleRate = 44.1e3;
static int iBlockSize = 512;
static std::string sOutputFileName = "ITAPA_Record.wav";
float fRecordingTime = 5; // Seconds


class Loopback : public ITADatasourceRealization {
public:
	Loopback(ITADatasource* pSource, long lChannels, double dSampleRate, long lBuffersize)
		: ITADatasourceRealization((unsigned int)lChannels, dSampleRate,
		(unsigned int)lBuffersize) {
		this->pSource = pSource;
		bFirst = true;
	}


	const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo) {
		if (bFirst) {
			for (unsigned int i = 0; i<m_uiChannels; i++)
				memcpy(GetWritePointer(i),
					pSource->GetBlockPointer(i, pStreamInfo),
					m_uiBlocklength * sizeof(float));
			IncrementWritePointer();
			pSource->IncrementBlockPointer();
			bFirst = false;
		}

		return ITADatasourceRealization::GetBlockPointer(uiChannel, pStreamInfo);
	}

	void IncrementBlockPointer() {
		ITADatasourceRealization::IncrementBlockPointer();
		bFirst = true;
	}

private:
	ITADatasource* pSource;
	bool bFirst;
};

void loopback() {
	ITAJACKInterface jack(iBlockSize);	
	jack.SetPlaybackEnabled(true);
	jack.SetRecordEnabled(true);

	auto err = jack.Initialize("jack-test");
	if (err != ITAJACKInterface::ITA_JACK_NO_ERROR) {
		std::cerr << "Failed to init jack!" << std::endl;
		return;
	}

	auto dsIn = jack.GetRecordDatasource();
	int blockSize = dsIn->GetBlocklength();

// TODO
	ITADatasource* pSource = NULL;

	try
	{
		pSource = new ITAFileDatasource("../ITAAsioTests/Trompete.wav", blockSize, true);
	}
	catch (ITAException& e)
	{
		std::cerr << "Could open audio file, error = " << e << std::endl;
		pSource = new ITAStreamFunctionGenerator(1, dSampleRate, blockSize, ITAStreamFunctionGenerator::SINE, 300, 0.9f, true);
	}

	

	Loopback dsLoop(pSource, jack.GetNumInputChannels(), jack.GetSampleRate(), dsIn->GetBlocklength());
	
	jack.SetPlaybackDatasource(&dsLoop);

	jack.Open();
	
	jack.Start();
	
	getchar();

	jack.Stop();

	jack.Close();
	jack.Finalize();

	return;
}

int main(int argc,char *argv[]) {
	std::cout << "Starting loopback ..." << std::endl;
	loopback();

	return 0;
}