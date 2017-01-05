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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM
#define INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM

#include <ITADataSourcesDefinitions.h>

#include <ITADataSource.h>
#include <ITASampleFrame.h>

#include <string>
#include <vector>

class CITANetAudioStreamingClient;

//! Network audio stream
/**
 * Audio streaming for a signal source that is connected via TCP/IP.
 *
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API CITANetAudioStream : public ITADatasource
{
public:
	CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity = 2048 );
	virtual ~CITANetAudioStream();

	bool Connect( const std::string& sAddress, int iPort );
	bool GetIsConnected() const;

	int GetRingBufferSize() const;
	
	unsigned int GetBlocklength() const;
	unsigned int GetNumberOfChannels() const;
	double GetSampleRate() const;
	const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* );
	void IncrementBlockPointer();

protected:
	//! This method is called by the streaming client and pushes sampes into the ring buffer
	/**
	  * \param sfNewSamples Sample buffer (multi channel) with sample data
	  * \param iNumSamples samples to be read from the sample frame (must be smaller or equal length)
	  *
	  * \return Number of free samples in ring buffer
	  */
	int Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples );

	int GetRingbufferFreeSamples();

private:
	CITANetAudioStreamingClient* m_pNetAudioStreamingClient;

	double m_dSampleRate;
	ITASampleFrame m_sfOutputStreamBuffer;

	int m_iReadCursor; //!< Cursor where samples will be consumed from ring buffer on next block
	int m_iWriteCursor; //!< Cursor where samples will feeded into ring buffer from net audio producer
	ITASampleFrame m_sfRingBuffer; //!< Buffer incoming data

	friend class CITANetAudioStreamingClient;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM
