#include <ITADataSourceUtils.h>

#include <ITADataSource.h>
#include <ITAStreamInfo.h>
#include <ITAAudiofileWriter.h>
#include <ITAException.h>
#include <ITAClock.h>

#include <cmath>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void WriteFromDatasourceToBuffer(ITADatasource* pSource,
				  		         float** ppfDest,
						         unsigned int uiNumberOfSamples,
						         double dGain,
							     bool bOnline,
								 bool bDisplayProgress) {
	// Nullzeiger ausschließen
	// TODO: Fehlerbehandlung
	if (!pSource) return;

	unsigned int uiChannels = pSource->GetNumberOfChannels();
	double dSamplerate = pSource->GetSampleRate();
	unsigned int uiBlocklength = pSource->GetBlocklength();
	ITAStreamInfo siState;

	long periodMs = (long)ceil(uiBlocklength / dSamplerate * 1000);

#ifdef _WIN32
	HANDLE hTimer=0;
	if (bOnline) {
		// Timer erzeugen
		if (FAILED(hTimer = CreateWaitableTimer(NULL, false, NULL)))
			ITA_EXCEPT1(UNKNOWN, "Timer konnte nicht erzeugt werden");

		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = 0;
		SetWaitableTimer(hTimer, &liDueTime, periodMs, NULL, NULL, true);
	}
#endif



	try {
		unsigned int n=0;
		float fProgress = 0.0f;
		while (n < uiNumberOfSamples) 
		{
#ifdef _WIN32
			// Warten
			if (bOnline) WaitForSingleObject(hTimer, INFINITE);
#else
			if (bOnline) usleep(periodMs * 1000);
#endif

			// Daten von der Quelle holen
			for (unsigned int i=0; i<uiChannels; i++) {
				const float* pfData = pSource->GetBlockPointer(i, &siState);

				unsigned int k = (uiNumberOfSamples - n);
				if (k > uiBlocklength) k = uiBlocklength;

				if( !pfData )
				{	// Stille einfügen
					for( unsigned int j = 0; j < uiBlocklength; j++ )
						ppfDest[ i ][ n + j ] = 0;
				}
				else
				{
					if( dGain == 1.0f )
						memcpy( ppfDest[ i ] + n, pfData, k * sizeof( float ) );
					else
						for( unsigned int j = 0; j < k; j++ )
							ppfDest[ i ][ n + j ] = ( float ) ( ( double ) pfData[ j ] * dGain );
				}
			}

			pSource->IncrementBlockPointer();
			n += uiBlocklength;

			siState.nSamples += uiBlocklength;
			siState.dStreamTimeCode = (double) (siState.nSamples) / dSamplerate;
			siState.dSysTimeCode = ITAClock::getDefaultClock()->getTime();

			if (bDisplayProgress) 
			{
				float p = 100 * (float) n / uiNumberOfSamples;
				if(p > fProgress + 5.0f)
				{
					fProgress = p;
					printf("WriteFromDatasourceToBuffer: %0.2f%% geschrieben", p);
				}
			}
		}
	} catch (...) {
#ifdef _WIN32
		if (bOnline) CloseHandle(hTimer);
#endif
		throw;
	}

	if (bDisplayProgress) 
		printf("WriteFromDatasourceToBuffer: 100,00%% geschrieben");

#ifdef _WIN32
	if (bOnline) 
		CloseHandle(hTimer);
#endif
}

void WriteFromDatasourceToFile(ITADatasource* pSource,
				  		       std::string sFilename,
						       unsigned int uiNumberOfSamples,
						       double dGain,
							   bool bOnline,
							   bool bDisplayProgress) {
	// Nullzeiger ausschließen
	// TODO: Fehlerbehandlung
	if (!pSource) return;

	unsigned int uiChannels = pSource->GetNumberOfChannels();
	unsigned int uiBlocklength = pSource->GetBlocklength();
	double dSamplerate = pSource->GetSampleRate();

	std::vector<float*> vpfData;
	for (unsigned int i=0; i<uiChannels; i++)
		vpfData.push_back(new float[uiBlocklength]);

	ITAAudiofileProperties props;
	props.iChannels = uiChannels;
	props.dSampleRate = dSamplerate;
	props.eQuantization = ITAQuantization::ITA_FLOAT;
	props.eDomain = ITADomain::ITA_TIME_DOMAIN;
	props.iLength = uiNumberOfSamples;
	ITAAudiofileWriter* writer = ITAAudiofileWriter::create(sFilename, props);
	ITAStreamInfo siState;

	long periodMs = (long)ceil(uiBlocklength / dSamplerate * 1000);

#ifdef _WIN32
	HANDLE hTimer=0;

	if (bOnline) {
		// Timer erzeugen
		if (FAILED(hTimer = CreateWaitableTimer(NULL, false, NULL))) {
			delete writer;
			ITA_EXCEPT1(UNKNOWN, "Timer konnte nicht erzeugt werden");
		}
		
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart=0;
		
		SetWaitableTimer(hTimer, &liDueTime, periodMs, NULL, NULL, true);
	}
#endif

	try {
		unsigned int n=0;
		float fProgress = 0.0;
		while (n < uiNumberOfSamples) {
#ifdef _WIN32
			// Warten
			if (bOnline) WaitForSingleObject(hTimer, INFINITE);
#else
			if (bOnline) usleep(periodMs * 1000);
#endif

			// Daten von der Quelle holen
			for (unsigned int i=0; i<uiChannels; i++) {
				const float* pfSource = pSource->GetBlockPointer(i, &siState);
				float* pfDest = vpfData[i];
				if (!pfSource)
					// Stille einfügen
					for (unsigned int j=0; j<uiBlocklength; j++) pfDest[j] = 0;
				else {
					if (dGain == 1)
						memcpy(pfDest, pfSource, uiBlocklength*sizeof(float));
					else 
						for (unsigned int j=0; j<uiBlocklength; j++)
							pfDest[j] = pfSource[j] * (float) dGain;
				}
			}
			pSource->IncrementBlockPointer();
	
			siState.nSamples += uiBlocklength;
			siState.dStreamTimeCode = (double) (siState.nSamples) / dSamplerate;
			siState.dSysTimeCode = ITAClock::getDefaultClock()->getTime();

			// Daten schreiben
			writer->write((std::min)(uiBlocklength, (uiNumberOfSamples - n)), vpfData);

			n += uiBlocklength;

			if (bDisplayProgress)
			{
				float p = 100 * (float) n / uiNumberOfSamples;
				if(p > fProgress + 5.0f)
				{
					fProgress = p;
					printf("WriteFromDatasourceToFile: %0.2f%% geschrieben\r", p);
				}
			}
		}
	} catch (...) {
#ifdef _WIN32
		if (bOnline) CloseHandle(hTimer);
#endif
		delete writer;
		throw;
	}

	if (bDisplayProgress) 
	printf("WriteFromDatasourceToFile: 100,00%% geschrieben\r");

#ifdef _WIN32
	if (bOnline) 
		CloseHandle(hTimer);
#endif

	delete writer;
}

