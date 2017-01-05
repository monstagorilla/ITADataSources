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

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <ITASampleFrame.h>

class ITADatasource;
class CITANetAudioServer;
class CITANetAudioMessage;
class VistaTCPSocket;

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
	virtual ~CITANetAudioStreamingServer() {};

	bool Start( const std::string& sAddress, int iPort );
	bool IsClientConnected() const;
	std::string GetNetworkAddress() const;
	int GetNetworkPort() const;
	void Stop();

	void SetInputStream( ITADatasource* pInStream );
	
	int GetNetStreamBlocklength() const;
	int GetNetStreamNumberOfChannels() const;
	double GetNetStreamSampleRate() const;

	void SetAutomaticUpdateRate();

	bool LoopBody();

protected:
	int Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples );
	ITADatasource* GetInputStream() const;

private:
	CITANetAudioServer* m_pNetAudioServer;
	ITASampleFrame m_sfTempTransmitBuffer;
	ITADatasource* m_pInputStream;
	VistaConnectionIP* m_pConnection;

	CITANetAudioMessage* m_pMessage;

	int m_iUpdateStrategy;
	int m_iClientRingBufferFreeSamples;

	friend class CITANetAudioServer;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
