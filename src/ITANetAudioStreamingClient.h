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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT
#define INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT

#include <ITADataSourcesDefinitions.h>

#include <ITASampleFrame.h>
#include <ITAStreamProbe.h>
#include <ITAStopWatch.h>

#include <VistaInterProcComm/Concurrency/VistaThreadEvent.h>
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>

#include <string>
#include <vector>

class CITANetAudioClient;
class CITANetAudioMessage;
class CITANetAudioProtocol;
class CITANetAudioStream;
class ITABufferedDataLoggerImplClient;
class VistaConnectionIP;

//! Network audio streaming client
/**
 * Audio streaming for a signal source that is connected via TCP/IP or UDP.
 * Implements the ITA network protocol for audio streaming on client side.
 *
 * @todo: move to src folder
 *
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API CITANetAudioStreamingClient : public VistaThreadLoop
{
public:
	CITANetAudioStreamingClient( CITANetAudioStream* pParent );
	virtual ~CITANetAudioStreamingClient();

	bool Connect( const std::string& sAddress, const int iPort, const bool bUseUDP );
	bool GetIsConnected() const;
	void Disconnect();

	bool LoopBody();

	std::string GetClientLoggerBaseName() const;
	void SetClientLoggerBaseName( const std::string& );

	void SetDebuggingEnabled( bool bEnabled );
	bool GetIsDebuggingEnabled() const;

protected:
	void TriggerBlockIncrement();

private:
	CITANetAudioClient* m_pClient;
	CITANetAudioStream* m_pStream;

	CITANetAudioProtocol* m_pProtocol;
	CITANetAudioMessage* m_pMessage;
	VistaConnectionIP* m_pConnection;
	
	ITASampleFrame m_sfReceivingBuffer; //!< Buffer incoming data
	
	bool m_bStopIndicated;
	bool m_bStopped;

	int m_iStreamingBlockId;

	double m_dServerClockSyncRequestTimeInterval;
	double m_dServerClockSyncLastSyncTime;

	ITABufferedDataLoggerImplClient* m_pClientLogger;
	std::string m_sClientLoggerBaseName;
	ITAStopWatch m_swTryReadBlockStats, m_swTryReadAccessStats;
	bool m_bDebuggingEnabled;

	friend class CITANetAudioStream;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_CLIENT
