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
class ITABufferedDataLoggerImplAudio;

//! Network audio stream
/**
  * Audio streaming for a signal source that is connected via TCP/IP.
  * The network audio stream behaves like a client and receives samples
  * from a network audio stream server, CITANetAudioStreamingSearver.
  * 
  * The stream will always work within a streaming context and will never
  * block the streaming processing, because it is decoupled from the 
  * network connection and forwards samples from an internal ring buffer.
  * If the buffer runs out of samples, zeros will be return. If the buffer
  * overruns, the sample server will be suspendeb by blocking the network
  * data flow.
  *
  * Latency can be managed by either providing a small ring buffer or
  * oversizing the ring buffer and requesting a target latency.
  *
  * \note not thread-safe
  */
class ITA_DATA_SOURCES_API CITANetAudioStream : public ITADatasource
{
public:
	//! Constructor of a network audio stream
	CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity = 2048 );
	virtual ~CITANetAudioStream();

	//! Network streaming status of client
	enum StreamingStatus
	{
		INVALID = -1, //!< Invalid status, for exception detection
		STOPPED, //!< Client not connected to a server and streaming stopped, i.e. not receiving samples by choice
		CONNECTED, //!< Client is connected to a sample server (and potentially receives samples)
		STREAMING, //!< 
		BUFFER_UNDERRUN,
		BUFFER_OVERRUN,
	};

	//! Connect a streaming server
	/**
	  * @sAddress[in] Server address IP, i.e. 127.0.0.1
	  * @iPort[in] Server socket port, defaults to 12480
	  */
	bool Connect( const std::string& sAddress, int iPort = 12480 );

	void Disconnect();

	//! Returns the connection status
	/**
	  * @return True, if connected
	  */
	bool GetIsConnected() const;

	//! Sets the minimal latency possible
	/**
	  * Real-time network audio is considered to process at lowest latency possible.
	  * However, this implementation requires at least one block. Hence latency is
	  * depending on sampling rate and block length.
	  *
	  * @sa GetMinimumLatencySamples()
	  * @sa GetMinimumLatencySamples()
	  */
	float GetMinimumLatencySeconds() const;

	float GetMaximumLatencySeconds() const;
	int GetMinimumLatencySamples() const;
	int GetMaximumLatencySamples() const;

	//! Returns the NetAudio streaming logger base name
	std::string GetNetAudioStreamLoggerBaseName() const;

	//! Sets the NetAudio streaming logger base name
	void SetNetAudioStreamingLoggerBaseName( const std::string& sBaseName );

	//! Returns (static) size of ring buffer
	/**
	  * @return Number of maximum samples that can be hold by internal ring buffer
	  */
	int GetRingBufferSize() const;

	//! Returns true if ring buffer is full
	/**
	  * @return True, if ring buffer full (down to the last sample, not smaller that block size)
	  */
	bool GetIsRingBufferFull() const;

	//! Returns true if ring buffer is empty
	/**
	  * @return True, if ring buffer empty (down to the last sample, not smaller that block size)
	  */
	bool GetIsRingBufferEmpty() const;
	
	//! Returns block size
	unsigned int GetBlocklength() const;

	//! Returns number of channels
	unsigned int GetNumberOfChannels() const;

	//! Returns sampling rate
	double GetSampleRate() const;

	//! Audio streaming block read for a given channel
	/** 
	  * This method is called by the streaming context for each channel, also providing stream infos.
	  * It copies samples out of the ring buffer, but does not touch the read/write cursors.
	  */
	const float* GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* );
	
	//! Audio streaming block increment
	/**
	  * This method updates the read curser and forwards it by one block. This operations
	  * frees samples out of the ring buffer. It also triggersan event
	  * to indicate that the ring buffer readable sample number has been decreased.
	  * Depending on the network streaming settings, the trigger will be forwarded
	  * to the server to inform about the number of free samples, that can be re-filled.
	  */
	void IncrementBlockPointer();


protected:
	//! This method is called by the networkg client and pushes samples into the ring buffer
	/**
	  * If samples fit ring buffer, the rite curser will be increased by iNumSamples, hence
	  * the number of free (writable) samples decreases.
	  *
	  * @param[in] sfNewSamples Sample frame with new samples to be appended to the ring buffer
	  * \param iNumSamples samples to be read from the sample frame (must be smaller or equal length)
	  *
	  * @return Number of free samples in ring buffer
	  *
	  * @note This method is not called out of the audio streaming context but out of the network context.
	  */
	int Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples );

	//! Returns samples that can be read from ring buffer
	/**
	* @return Readable samples between read and write cursor.
	*/
	int GetRingBufferAvailableSamples() const;

	//! Returns free samples between write and read cursor
	/**
	* @return Free samples between write and read cursor.
	*/
	int GetRingBufferFreeSamples() const;

private:
	CITANetAudioStreamingClient* m_pNetAudioStreamingClient; //!< Audio streaming network client

	double m_dSampleRate; //!< Sampling rate
	ITASampleFrame m_sfOutputStreamBuffer; //!< Output samples temp buffer (audio context)

	int m_iReadCursor; //!< Cursor where samples will be consumed from ring buffer on next block
	int m_iWriteCursor; //!< Cursor where samples will be fed into ring buffer from net audio producer (always ahead)
	bool m_bRingBufferFull; //!< Indicator if ring buffer is full (and read cursor equals write cursor)
	ITASampleFrame m_sfRingBuffer; //!< Ring buffer

	int m_iStreamingStatus; //!< Current streaming status
	double m_dLastStreamingTimeCode;

	ITABufferedDataLoggerImplStream* m_pAudioStreamLogger; //!< Logging for the audio stream
	ITABufferedDataLoggerImplNet* m_pNetworkStreamLogger; //!< Logging for the network stream
	std::string m_sNetAudioStreamLoggerBaseName;

	int iAudioStreamingBlockID; //!< Audio streaming block id
	int iNetStreamingBlockID; //!< Network streaming block id
	
	friend class CITANetAudioStreamingClient;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAM
