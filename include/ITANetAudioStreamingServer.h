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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
#define INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER

#include <ITADataSourcesDefinitions.h>

#include <ITASampleFrame.h>
#include <ITAStopWatch.h>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>

#include <string>
#include <vector>
#include <iostream>


class ITADatasource;
class CITANetAudioMessage;
class CITANetAudioProtocol;
class CITANetAudioServer;
class CITANetAudioMessage;
class VistaTCPSocket;
class ITABufferedDataLoggerImplServer;
class VistaConnectionIP;


//! Network audio streaming server (for connecting a net audio stream) with an ITADataSource connection
/**
  * Audio sample transmitter for a networked signal source that can connect via TCP/IP.
  *
  * @sa CITANetAudioStream, CITANetAudioSampleServer
  * @note not thread-safe
  */
class ITA_DATA_SOURCES_API CITANetAudioStreamingServer : public VistaThreadLoop
{
public:

	CITANetAudioStreamingServer();
	~CITANetAudioStreamingServer();

	//! Start to listen on a socket (blocking)
	bool Start( const std::string& sAddress, const int iPort, const double dTimeIntervalCientSendStatus );
	bool IsClientConnected() const;
	std::string GetNetworkAddress() const;
	int GetNetworkPort() const;

	void Stop();

	void SetInputStream( ITADatasource* pInStream );

	int GetNetStreamBlocklength() const;
	int GetNetStreamNumberOfChannels() const;
	double GetNetStreamSampleRate() const;


	//! Enabled/disables export of loggers
	void SetDebuggingEnabled( bool bEnabled );

	//! Logging export flag getter
	bool GetIsDebuggingEnabled() const;

	int GetSendingBlockLength() const;
	void SetSendingBlockLength( const int iSendingBlockLength );

	void SetTargetLatencySamples( const int iTargetLatency );
	int GetTargetLatencySamples() const;

	void SetServerLogBaseName( const std::string& sBaseName );
	std::string GetServerLogBaseName() const;

	bool LoopBody();

protected:
	ITADatasource* GetInputStream() const;

private:
	CITANetAudioServer* m_pNetAudioServer;
	ITASampleFrame m_sfTempTransmitBuffer;
	ITADatasource* m_pInputStream;
	VistaConnectionIP* m_pConnection;

	CITANetAudioMessage* m_pMessage;

	ITABufferedDataLoggerImplServer* m_pServerLogger;
	std::string m_sServerLogBaseName;
	ITAStopWatch m_swTryReadBlockStats, m_swTryReadAccessStats;
	bool m_bDebuggingEnabled;

	int m_iServerBlockId;
	double m_dLastTimeStamp;

	int m_iTargetLatencySamples;
	int m_iEstimatedClientRingBufferFreeSamples;
	int m_iClientRingBufferSize;
	int m_iSendingBlockLength;
	int m_iMaxSendBlocks;

	friend class CITANetAudioServer;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
