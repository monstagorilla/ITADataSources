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

#include <ITANetAudioProtocol.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <ITANetAudioProtocol.h>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <ITASampleFrame.h>

class ITADatasource;
class CITANetAudioMessage;
class CITANetAudioProtocol;
class CITANetAudioServer;
class CITANetAudioMessage;
class VistaTCPSocket;
class ITABufferedDataLoggerImplServer;


//! Network audio sample server (for connecting a net audio stream)
/**
 * Audio sample transmitter for a networked signal source that can connect via TCP/IP.
 *
 * \sa CITANetAudioStream
 * \note not thread-safe
 */
class ITA_DATA_SOURCES_API CITANetAudioStreamingServer : public VistaThreadLoop
{
public:

	enum UpdateStrategy
	{
		AUTO = 1, //!< Automatic update rate based on sample rate and block length of client (default)
		ADAPTIVE, //!< Adaptive update rate, adjusts for drifting clocks
		CONSTANT, //!< Set a user-defined update rate (may cause forced pausing of sample feeding or dropouts on client side)
	};

	CITANetAudioStreamingServer();
	~CITANetAudioStreamingServer();

	bool Start(const std::string& sAddress, int iPort, double dTimeIntervalCientSendStatus);
	bool IsClientConnected() const;
	std::string GetNetworkAddress() const;
	int GetNetworkPort() const;
	bool LoopBody();
	void Stop();

	void SetInputStream( ITADatasource* pInStream );
	
	int GetNetStreamBlocklength() const;
	int GetNetStreamNumberOfChannels() const;
	double GetNetStreamSampleRate() const;

	void SetAutomaticUpdateRate();

protected:
	ITADatasource* GetInputStream() const;

private:
	CITANetAudioServer* m_pNetAudioServer;
	ITASampleFrame m_sfTempTransmitBuffer;
	ITADatasource* m_pInputStream;
	VistaConnectionIP* m_pConnection;

	CITANetAudioProtocol::StreamingParameters m_oServerParams;
	CITANetAudioMessage* m_pMessage;

	int iServerBlockId;
	ITABufferedDataLoggerImplServer* m_pServerLogger;

	int m_iUpdateStrategy;
	int m_iClientRingBufferFreeSamples;
	int m_iMaxSendBlocks;
	double m_dLastTimeStamp;

	friend class CITANetAudioServer;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
