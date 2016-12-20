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

#include <ITANetAudioProtocol.h>

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
	CITANetAudioServer();
	virtual ~CITANetAudioServer();

	std::string GetServerAddress() const;
	int GetNetworkPort() const;
	bool Start(const std::string& sAddress, int iPort);
	VistaConnectionIP* GetConnetion() const;

	void Disconnect(); 
	bool IsConnected() const; 


private:
	VistaTCPServer* m_pServer;
	VistaTCPSocket* m_pSocket;
	VistaConnectionIP* m_pConnection;

	int m_iServerPort;
	std::string m_sServerAddress;

};
#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_SERVER
