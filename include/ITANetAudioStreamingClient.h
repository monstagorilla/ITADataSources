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

#include <ITANetAudioProtocol.h>

#include <ITASampleFrame.h>

#include <VistaInterProcComm/Concurrency/VistaThreadEvent.h>
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>

#include <string>
#include <vector>

class CITANetAudioClient;
class CITANetAudioMessage;
class CITANetAudioProtocol;
class CITANetAudioStream;

//! Network audio streaming client
/**
 * Audio streaming for a signal source that is connected via TCP/IP.
 * Implements the ITA network protocol for audio streaming in client side.
 *
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API CITANetAudioStreamingClient : public VistaThreadLoop
{
public:
	CITANetAudioStreamingClient( CITANetAudioStream* pParent );
	virtual ~CITANetAudioStreamingClient();

	bool Connect( const std::string& sAddress, int iPort );
	bool GetIsConnected() const;
	void Disconnect();

	bool LoopBody();

protected:
	void TriggerBlockIncrement();

private:
	CITANetAudioClient* m_pClient;
	CITANetAudioStream* m_pStream;

	CITANetAudioProtocol* m_pProtocol;
	CITANetAudioMessage* m_pMessage;
	VistaConnectionIP* m_pConnection;

	VistaThreadEvent m_oBlockIncrementEvent;

	ITASampleFrame m_sfReceivingBuffer; //!< Buffer incoming data

	CITANetAudioProtocol::StreamingParameters m_oClientParams;
	CITANetAudioProtocol::StreamingParameters m_oServerParams;

	bool m_bStopIndicated;

	friend class CITANetAudioStream;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT
