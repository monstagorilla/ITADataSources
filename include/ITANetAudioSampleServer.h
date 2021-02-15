/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2021
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


//! Sample-generation class with abstract method for providing samples
/*
 * This ready-to-use class helps to provide samples for a NetAudio streaming server with
 * a single method for processing that has to be implemented ...
 * ... just derive and implement Process() method. Have a look at Zero() method
 * for exemplary usage of sample buffer.
 */ 
class CITASampleProcessor : public ITADatasourceRealization
{
public:
	//! Create a sample processor with streaming parameters
	/*
	 * @param[in] iNumChannels Channels provided
	 * @param[in] dSampleRate Audio processing sampling rate
	 * @param[in] iBlockLength Audio processing block length / buffer size
	 */ 
	inline CITASampleProcessor( const int iNumChannels, const double dSampleRate, const int iBlockLength )
		: ITADatasourceRealization( ( unsigned int ) ( iNumChannels ), dSampleRate, ( unsigned int ) ( iBlockLength ) )
	{
		m_vvfSampleBuffer.resize( iNumChannels );
		for( size_t c = 0; c < iNumChannels; c++ )
			m_vvfSampleBuffer[ c ].resize( iBlockLength );

		Zero();
	};

	inline ~CITASampleProcessor()
	{
	};

	//! Sets all channels and samples to zero
	inline void Zero()
	{
		/* 
		 * Use this as an example how to work with the buffer structure.
		*/
		
		// Iterate over channels
		for( size_t c = 0; c < m_vvfSampleBuffer.size(); c++ ) 
		{
			std::vector< float >& vfSingleChannelSampleBuffer( m_vvfSampleBuffer[ c ] ); // One channel
			
			// Iterate over samples of channel
			for( size_t n = 0; n < vfSingleChannelSampleBuffer.size(); n++ )
			{
				float& fSample( vfSingleChannelSampleBuffer[ n ] ); // One sample
				fSample = 0.0f; // -> Manipulation
			}
		}
	};
	
	//! Process samples (overwrite this virtual method)
	/**
	  * Method that is called in audio streaming context and requests
	  * to produce or copy audio samples into the internal buffer m_vvfSampleBuffer
	  *
	  * @param[in] pStreamInfo Information over streaming status, i.e. sample count and time stamp
	  *
	  */
	virtual void Process( const ITAStreamInfo* pStreamInfo ) =0;

protected:
	std::vector< std::vector< float > > m_vvfSampleBuffer; //!< Multi-channel sample buffer to be filled

private:
	//! Delegate internal buffer to audio stream (ITADatasource)
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

//! Network audio sample server (for providing samples via derived generator class)
/**
  * Audio sample transmitter for a networked sample callback function that can connect via TCP/IP.
  *
  * @sa CITANetAudioStream CITANetAudioStreamingServer CITASampleProcessor
  * @note not thread-safe
  */
class CITANetAudioSampleServer : public CITANetAudioStreamingServer
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
