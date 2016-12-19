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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT
#define INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT

#include <ITADataSourcesDefinitions.h>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <ITADataSource.h>
#include <ITASampleFrame.h>

#include <string>
#include <vector>

class CITANetAudioClient;
class CITANetAudioStream;

//! Network audio streaming client
/**
 * Audio streaming for a signal source that is connected via TCP/IP.
 *
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API CITANetAudioStreamingClient : public VistaThreadLoop
{
public:
	CITANetAudioStreamingClient( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity );
	virtual ~CITANetAudioStreamingClient();

	bool Connect( const std::string& sAddress, int iPort );
	bool GetIsConnected() const;

	bool LoopBody();

private:
	CITANetAudioClient* m_pClient;
	CITANetAudioStream* m_pStream;

	ITASampleFrame m_sfReceivingBuffer; //!< Buffer incoming data

	friend class CITANetAudioClient;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT
