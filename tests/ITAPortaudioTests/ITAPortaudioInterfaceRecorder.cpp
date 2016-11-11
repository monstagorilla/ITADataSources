// $Id: ITAPortaudioInterfaceRecorder.cpp 2333 2012-03-05 14:21:39Z stienen $

#include <iostream>
#include <string>
#include <ITADatasourceUtils.h>
#include <ITAPortaudioInterface.h>

static double dSampleRate = 44.1e3;
static int iBlockSize = 512;
static std::string sOutputFileName = "ITAPA_Record.wav";
float fRecordingTime = 5; // Seconds

void record() {
	ITAPortaudioInterface ITAPA(dSampleRate, iBlockSize);

	int iInputDevice = ITAPA.GetDefaultInputDevice();
	//iInputDevice = 4;
	
	ITAPortaudioInterface::ITA_PA_ERRORCODE err;
	err = ITAPA.Initialize(iInputDevice);
	ITAPA.SetPlaybackEnabled(false);
	ITAPA.SetRecordEnabled(true);
	ITADatasource* pdsRecordDatasource = ITAPA.GetRecordDatasource();
	ITAPA.Open();
	
	ITAPA.Start();
	WriteFromDatasourceToFile(pdsRecordDatasource, sOutputFileName, (unsigned int)(5*dSampleRate), 1.0, true, true);
	ITAPA.Stop();

	ITAPA.Close();
	ITAPA.Finalize();

	return;
}

int main(int argc,char *argv[]) {
	std::cout << "Starting recording ..." << std::endl;
	record();

	return 0;
}