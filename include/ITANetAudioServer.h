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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_SERVER
#define INCLUDE_WATCHER_ITA_NET_AUDIO_SERVER

#include <ITADataSourcesDefinitions.h>

#include <ITASampleFrame.h>


#include <string>
#include <vector>

class VistaTCPSocket;
class CITANetAudioStreamingServer;
class VistaTCPServer;

//! Realizes server functionality for network audio streaming
/**
  * Can be connected to an ITADataSource as a streaming source
  * or to a user-implemented sample producer, i.e. an audio sythesizer.
  */
class ITA_DATA_SOURCES_API CITANetAudioServer
{
public:
	CITANetAudioServer( CITANetAudioStreamingServer* pParent );
	virtual ~CITANetAudioServer();

	std::string GetServerAddress() const;
	int GetNetworkPort() const;
	double GetClientSampleRate() const;
	bool Start( const std::string& sAddress, int iPort );
	void Disconnect(); 
	bool IsConnected() const; 
	bool LoopBody();


private:
	VistaTCPServer* m_pServer;
	VistaTCPSocket* m_pSocket;

	int m_iServerPort;
	std::string m_sServerAddress;

	CITANetAudioStreamingServer* m_pParent;

	ITASampleFrame m_sfReceivingBuffer;

	bool m_bStopIndicated;

};
#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_SERVER
