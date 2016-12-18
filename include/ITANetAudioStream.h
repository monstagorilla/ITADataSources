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
	CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity );
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
	int Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples );

private:
	CITANetAudioStreamingClient* m_pNetAudioProducer;

	double m_dSampleRate;
	ITASampleFrame m_sfOutputStreamBuffer;

	int m_iReadCursor; //!< Cursor where samples will be consumed from ring buffer on next block
	int m_iWriteCursor; //!< Cursor where samples will feeded into ring buffer from net audio producer
	ITASampleFrame m_sfRingBuffer;

	friend class CITANetAudioStreamingClient;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM
