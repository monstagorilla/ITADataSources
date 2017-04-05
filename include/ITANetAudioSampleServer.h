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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER
#define INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER

#include <ITADataSourcesDefinitions.h>
#include <ITANetAudioStreamingServer.h>
#include <ITADataSourceRealization.h>


//! Sample-generation callback function pointer class
class CITASampleProcessor : public ITADatasourceRealization
{
public:
	inline CITASampleProcessor( const double dSampleRate, const int iNumChannels, const int iBlockLength )
		: ITADatasourceRealization( ( unsigned int ) ( iNumChannels ), dSampleRate, ( unsigned int ) ( iBlockLength ) )
	{
		for( size_t c = 0; c < iNumChannels; c++ )
		{
			m_vvfSampleBuffer.push_back( std::vector< float >() );
			for( size_t n = 0; n < iBlockLength; n++ )
				m_vvfSampleBuffer[ c ].push_back( 0.0f );
		}
	};

	inline ~CITASampleProcessor() {};

	//! Process samples (overwrite this virtual method)
	/**
	  * Method that is called in audio streaming context and requests
	  * to produce or copy audio samples into the internal buffer m_vvfSampleBuffer
	  *
	  * @param[in] pStreamInfo Information over streaming status, i.e. sample count and time stamp
	  */
	virtual void Process( const ITAStreamInfo* pStreamInfo ) =0;

protected:
	std::vector< std::vector< float > > m_vvfSampleBuffer; //!< Multi-channel sample buffer

public:
	//! Delegate internal buffer to audio stream
	inline void ProcessStream( const ITAStreamInfo* pInfo )
	{
		Process( pInfo );

		for( size_t c = 0; c < m_vvfSampleBuffer.size(); c++ )
		{
			float* pfData = GetWritePointer( ( unsigned int ) ( c ) );
			for( size_t n = 0; n < m_vvfSampleBuffer[ c ].size(); n++ )
				pfData[ n ] = m_vvfSampleBuffer[ c ][ n ];
		}

		IncrementWritePointer();
	};
};

//! Network audio sample server (for providing samples via individual callback)
/**
  * Audio sample transmitter for a networked sample callback function that can connect via TCP/IP.
  *
  * @sa CITANetAudioStream CITANetAudioStreamingServer
  * @note not thread-safe
  */
class ITA_DATA_SOURCES_API CITANetAudioSampleServer : public CITANetAudioStreamingServer
{
public:
	inline CITANetAudioSampleServer( CITASampleProcessor* pProcessor )
		: m_pSampleProcessor( pProcessor )
	{
		SetInputStream( m_pSampleProcessor );
	};

	inline ~CITANetAudioSampleServer()
	{};

private:
	//! Prohibit public access to streaming context and delegate
	inline void SetInputStream( ITADatasource* pDataSource )
	{
		CITANetAudioStreamingServer::SetInputStream( pDataSource );
	};

	//! Prohibit public access to streaming context and delegate
	inline ITADatasource* GetInputStream() const
	{
		return CITANetAudioStreamingServer::GetInputStream();
	};

	CITASampleProcessor* m_pSampleProcessor; //!< Callback / sample processor
};


#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER
