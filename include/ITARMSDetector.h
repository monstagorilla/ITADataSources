/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2016
*
* ----------------------------------------------------------------
*				    ____  __________  _______
*				   //  / //__   ___/ //  _   |
*				  //  /    //  /    //  /_|  |
*				 //  /    //  /    //  ___   |
*				//__/    //__/    //__/   |__|
*
* ----------------------------------------------------------------
*
*/

#ifndef INCLUDE_WATCHER_ITA_RMS_DETECTOR
#define INCLUDE_WATCHER_ITA_RMS_DETECTOR

#include <ITADataSourcesDefinitions.h>

#include <ITACriticalSection.h>
#include <ITADataSource.h>
#include <vector>

//! Detector for RMS values
/**
 *
 * \note Die Klasse ist Thread-safe
 */
class ITA_DATA_SOURCES_API ITARMSDetector : public ITADatasource
{
public:
	ITARMSDetector( ITADatasource* pDatasource );
	virtual ~ITARMSDetector();
	inline ITADatasource* GetDatasource() const
	{
		return m_pDataSource;
	};

	void Reset();
	void GetOverallRMS( float* pfPeak, unsigned int* puiChannel = 0, bool bReset = true );
	void GetOverallRMSDecibel( double* pdPeakDecibel, unsigned int* puiChannel = 0, bool bReset = true );
	float GetRMS( unsigned int uiChannel, bool bReset = true );
	double GetRMSDecibel( unsigned int uiChannel, bool bReset = true );
	void GetRMSs( float* pfDest, bool bReset = true );
	void GetRMSs( std::vector<float>& vfDest, bool bReset = true );
	void GetRMSsDecibel( std::vector<double>& vdDestDecibel, bool bReset = true );
	
	inline unsigned int GetBlocklength() const 
	{ 
		return m_uiBlocklength;
	};

	inline unsigned int GetNumberOfChannels() const
	{ 
		return m_uiChannels;
	};

	inline double GetSampleRate() const 
	{ 
		return m_dSamplerate;
	};

	virtual const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo );
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDataSource;			// Angeschlossene Datenquelle
	double m_dSamplerate;					// Abtastrate [Hz]
	unsigned int m_uiChannels;				// Anzahl Kanäle
	unsigned int m_uiBlocklength;			// Streaming Puffergröße [Samples]
	ITACriticalSection m_cs;				// Sichert exklusiven Zugriff auf die Daten (s.u.)
	float* m_pfRMSs;						// Spitzenwerte der einzelnen Kanäle
	float m_fOverallRMS;					// Spitzenwert über alle Kanäle
	unsigned int m_uiOverallRMSChannel;	// Kanal in dem der Spitzenwert auftrat
};

#endif // INCLUDE_WATCHER_ITA_RMS_DETECTOR