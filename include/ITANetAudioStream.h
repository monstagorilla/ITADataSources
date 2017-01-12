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
#include <iostream>
#include <fstream>
using namespace std;


class CITANetAudioStreamingClient;
class ITABufferedDataLoggerImplStream;
class ITABufferedDataLoggerImplNet;

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

	enum StreamingStatus
	{
		INVALID = -1,
		STOPPED,
		CONNECTED,
		STREAMING,
		BUFFER_UNDERRUN,
	};

	bool Connect( const std::string& sAddress, int iPort );
	bool GetIsConnected() const;

	//! Returns (static) size of ring buffer
	int GetRingBufferSize() const;

	//! Returns true if ring buffer is full
	bool GetIsRingBufferFull() const;

	//! Returns true if ring buffer is empty
	bool GetIsRingBufferEmpty() const;
	
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

	//! Returns free samples between write and read cursor
	int GetRingBufferFreeSamples() const;

private:
	CITANetAudioStreamingClient* m_pNetAudioStreamingClient;

	double m_dSampleRate;
	ITASampleFrame m_sfOutputStreamBuffer;

	int m_iReadCursor; //!< Cursor where samples will be consumed from ring buffer on next block
	int m_iWriteCursor; //!< Cursor where samples will be fed into ring buffer from net audio producer (always ahead)
	bool m_bRingBufferFull; //!< Indicator if ring buffer is full (and read cursor equals write cursor)
	ITASampleFrame m_sfRingBuffer; //!< Buffer incoming data

	int m_iStreamingStatus; //!< Current streaming status

	friend class CITANetAudioStreamingClient;
	ITABufferedDataLoggerImplStream* m_pStreamLogger;
	ITABufferedDataLoggerImplNet* m_pNetLogger;
	int iID;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM
