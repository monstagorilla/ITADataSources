// $Id: ITAPortaudioInterfaceTest.cpp 2333 2012-03-05 14:21:39Z stienen $

#include <string.h>
#include <stdio.h>
#include <iostream>

#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamFunctionGenerator.h>
#include <ITAStreamMultiplier1N.h>

using namespace std;

int main(int , char** )
{

	double dSampleRate = 44.1e3;
	int iBlockSize = 256;

	ITADatasource* pSource = NULL;
	
	// Data source (try Bauer.wav, else use sine signal)
	try
	{
		pSource = new ITAFileDatasource("../ITAAsioTests/Trompete.wav", iBlockSize, true);
	}
	catch (ITAException& e)
	{
		cerr << "Could open audio file, error = " << e << endl;
		pSource = new ITAStreamFunctionGenerator(1, dSampleRate, iBlockSize, ITAStreamFunctionGenerator::SINE, 300, 0.9f, true);
	}
		
	// Instantiate
	ITAPortaudioInterface ITAPA(dSampleRate, iBlockSize);
	ITAPortaudioInterface::ITA_PA_ERRORCODE err;

	// Initialize
	int iDefaultDriverID = ITAPA.GetDefaultOutputDevice();
	cout << "Initializing default device with ID " << iDefaultDriverID << endl;
	if ((err = ITAPA.Initialize(iDefaultDriverID)) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("* Successfully initialized Portaudio.\n");
	else
		printf("* Failed to initialize Portaudio: %s\n", ITAPA.GetErrorCodeString(err).c_str());

	int iDefaultOutputDevice = ITAPA.GetDefaultOutputDevice();
	int iNumOutputChannels = ITAPA.GetNumOutputChannels(iDefaultOutputDevice);

	ITAStreamMultiplier1N multiplier(pSource, iNumOutputChannels);


	// Attach data source
	if ((err = ITAPA.SetPlaybackDatasource(&multiplier)) ==  ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("* Successfully connected data source to ITAPortaudio.\n");
	else
		printf("* Failed to connect data source to ITAPortaudio: %s\n", ITAPA.GetErrorCodeString(err).c_str());


	// Open
	if ((err = ITAPA.Open()) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("Successfully opened a Portaudio stream.\n");
	else
		printf("Failed to open a Portaudio stream: %s\n", ITAPA.GetErrorCodeString(err).c_str());


	// Device infos
	for (int i = 0; i < ITAPA.GetNumDevices(); i++)
	{
		cout << "[" << i << "] ";
		if (iDefaultDriverID == i) 
			cout << "(default) ";
		cout << ITAPA.GetDeviceName(i) << "(" << ITAPA.GetNumOutputChannels(i) << " out, "
			<< ITAPA.GetNumInputChannels(i) << " in)" << endl;
	}
	
	// Start
	if ((err = ITAPA.Start()) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("Successfully started Portaudio streaming.\n");
	else
		printf("Failed to start Portaudio streaming: %s\n", ITAPA.GetErrorCodeString(err).c_str());
	

	// Playback
	float seconds = 10.0f;
	printf("... sleeping for %0.2f seconds\n", seconds);
	ITAPA.Sleep(seconds);
	

	// Stop
	if ((err = ITAPA.Stop()) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("* Successfully stopped Portaudio streaming.\n");
	else
		printf("* Failed to stop Portaudio streaming: %s\n", ITAPA.GetErrorCodeString(err).c_str());


	// Close
	if ((err = ITAPA.Close()) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("* Successfully closed Portaudio stream.\n");
	else
		printf("* Failed to close Portaudio stream: %s\n", ITAPA.GetErrorCodeString(err).c_str());
	

	// Finalize
	if ((err = ITAPA.Finalize()) == ITAPortaudioInterface::ITA_PA_NO_ERROR)
		printf("* Successfully finalized Portaudio.\n");
	else
		printf("* Failed to finalize Portaudio: %s\n", ITAPA.GetErrorCodeString(err).c_str());

	delete pSource;

	return 0;
}
