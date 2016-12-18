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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
#define INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER

#include <ITADataSourcesDefinitions.h>

#include <ITASampleFrame.h>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>

#include <string>
#include <vector>

class VistaTCPSocket;
class CITANetAudioSampleServer;
class VistaTCPServer;

//! Realizes server functionality for network audio streaming
/**
  * Can be connected to an ITADataSource as a streaming source
  * or to a user-implemented sample producer, i.e. an audio sythesizer.
  */
class CITANetAudioStreamingServer : public VistaThreadLoop
{
public:
	CITANetAudioStreamingServer( CITANetAudioSampleServer* pParent );
	virtual ~CITANetAudioStreamingServer();

	std::string GetServerAddress() const;
	int GetNetworkPort() const;
	bool Start( const std::string& sAddress, int iPort );
	void Disconnect(); bool IsConnected() const; bool LoopBody();

private:
	VistaTCPServer* m_pServer;
	VistaTCPSocket* m_pSocket;
	int m_iServerPort;
	std::string m_sServerAddress;
	CITANetAudioSampleServer* m_pParent;
	ITASampleFrame m_sfReceivingBuffer;

	bool m_bStopIndicated;

	int m_iClientChannels;
	int m_iClientRingBufferSize;
	int m_iClientBufferSize;
	int m_iClientRingBufferFreeSamples;
	double m_dClientSampleRate;
};
#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_STREAMING_SERVER
