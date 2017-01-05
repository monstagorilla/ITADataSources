/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2017
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

#ifndef INCLUDE_WATCHER_ITA_STREAM_PROBE
#define INCLUDE_WATCHER_ITA_STREAM_PROBE

#include <ITADataSourcesDefinitions.h>
#include <ITADataSource.h>

#include <ITASampleFrame.h>
#include <ITAAtomicPrimitives.h>
#include <ITAAudiofileCommon.h>

#include <string>
#include <vector>

// Vorwärtedeklarationen
class ITAAudiofileWriter;

//! A measuring sensor for audio streams
/**
  * Instanzen der Klasse ITAStreamProbe können als "Messspitze" in Audiostreams
  * eingehängt werden. Sie schreiben dann alle vorbeigeflossenen Daten in eine Audiodatei.
  */
class ITA_DATA_SOURCES_API ITAStreamProbe : public ITADatasource
{
public:
	ITAStreamProbe( ITADatasource* pDatasource, const std::string& sFilename, ITAQuantization iQuantization = ITAQuantization::ITA_FLOAT );

	virtual ~ITAStreamProbe();

	inline ITADatasource* GetDatasource() const 
	{ 
		return m_pDataSource;
	}

	inline std::string GetFilename() const
	{
		return GetFilePath();
	};

	inline std::string GetFilePath() const
	{ 
		return m_sFilePath;
	};

	inline unsigned int GetNumRecordedSamples() const 
	{ 
		return m_iRecordedSamples;
	}
	
	unsigned int GetBlocklength() const;
	unsigned int GetNumberOfChannels() const;
	double GetSampleRate() const;

	virtual const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo );
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDataSource;		//!< Incoming data source

	double m_dSampleRate;				//!< Sampling rate [Hz]
	int m_iChannels;					//!< Number of channels
	int m_iBlockLength;					//!< Streaming buffer size [Samples]

	ITAAtomicInt m_iRecordedSamples;	//!< Number of recorded samples
	ITASampleFrame m_sfBuffer;
	std::vector< bool > m_vbDataPresent;
	
	std::string m_sFilePath;			//!< File path
	ITAAudiofileWriter* m_pWriter;

};

#endif // INCLUDE_WATCHER_ITA_STREAM_PROBE
