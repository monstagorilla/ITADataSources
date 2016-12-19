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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_CONNECTION
#define INCLUDE_WATCHER_ITA_NET_AUDIO_CONNECTION

#include <ITADataSourcesDefinitions.h>

#include <ITADataSource.h>
#include <ITASampleFrame.h>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>

#include <string>
#include <vector>

class CITANetAudioMessage;
class CITANetAudioProtocol;
class CITANetAudioStream;

class VistaConnectionIP;

class CITANetAudioClient : public VistaThreadLoop
{
public:

	//! Create an network audio client that feeds into a network audio stream
	/**
	  * \param pParent ITADataSource-compatible audio stream
	  */
	CITANetAudioClient( CITANetAudioStream* pParent );
	~CITANetAudioClient();

	bool Connect( const std::string& sAddress, int iPort );
	void Disconnect();
	bool GetIsConnected() const;

	bool LoopBody();

private:
	CITANetAudioStream* m_pParent;

	VistaConnectionIP* m_pConnection;
	CITANetAudioProtocol* m_pProtocol;
	CITANetAudioMessage* m_pMessage;

	ITASampleFrame m_sfReceivingBuffer;
	bool m_bStopIndicated;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_CONNECTION
