// $Id: ITAPortaudioInterfaceRecorder.cpp 2333 2012-03-05 14:21:39Z stienen $

#include <iostream>
#include <string>
#include <ITADataSourceUtils.h>
#include <ITAJACKInterface.h>
#include <ITADataSourceRealization.h>

#include <ITAFileDataSource.h>
#include <ITAStreamFunctionGenerator.h>

#include <ITAException.h>


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
	ITAJACKInterface jack;	

	auto err = jack.Initialize("jack-loopback");
	if (err != ITAJACKInterface::ITA_JACK_NO_ERROR) {
		std::cerr << "Failed to init jack:" << jack.GetErrorCodeString(err) << std::endl;
		return;
	}
	
	jack.printInfo();

	std::cout << "GetRecordDatasource..." << std::endl;
	auto dsIn = jack.GetRecordDatasource();
	int blockSize = jack.GetBlockSize();
	

	std::cout << "creating loopback" << std::endl;
	Loopback dsLoop(dsIn, jack.GetNumInputChannels(), jack.GetSampleRate(), blockSize);
	

	std::cout << "settings playback datasource" << std::endl;
	jack.SetPlaybackDatasource(dsIn);

	std::cout << "open..." << std::endl;
	jack.Open();
	std::cout << "start..." << std::endl;
	jack.Start();
	
	std::cout << "Started. Press any key to quit." << std::endl;
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