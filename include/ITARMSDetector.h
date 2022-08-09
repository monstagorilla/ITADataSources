/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2022
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

#include <ITACriticalSection.h>
#include <ITADataSource.h>
#include <ITADataSourcesDefinitions.h>
#include <vector>

//! Detector for RMS values
/**
 *
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API ITARMSDetector : public ITADatasource
{
public:
	ITARMSDetector( ITADatasource* pDatasource );
	virtual ~ITARMSDetector( );
	inline ITADatasource* GetDatasource( ) const { return m_pDataSource; };

	void Reset( );
	void GetOverallRMS( float* pfRMS, unsigned int* puiChannel = 0, bool bReset = true );
	void GetOverallRMSDecibel( double* pdRMSDecibel, unsigned int* puiChannel = 0, bool bReset = true );
	float GetRMS( unsigned int uiChannel, bool bReset = true );
	double GetRMSDecibel( unsigned int uiChannel, bool bReset = true );
	void GetRMSs( std::vector<float>& vfDest, bool bReset = true );
	void GetRMSsDecibel( std::vector<double>& vdDestDecibel, bool bReset = true );

	inline unsigned int GetBlocklength( ) const { return m_uiBlocklength; };

	inline unsigned int GetNumberOfChannels( ) const { return m_uiChannels; };

	inline double GetSampleRate( ) const { return m_dSampleRate; };

	virtual const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo );
	virtual void IncrementBlockPointer( );

protected:
	ITADatasource* m_pDataSource;
	double m_dSampleRate;
	unsigned int m_uiChannels;
	unsigned int m_uiBlocklength;
	ITACriticalSection m_cs;
	float* m_pfRMSs;
	float m_fOverallRMS;
	unsigned int m_uiOverallRMSChannel;
};

#endif // INCLUDE_WATCHER_ITA_RMS_DETECTOR