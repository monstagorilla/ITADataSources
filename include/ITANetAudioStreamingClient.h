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

class CITANetAudioStream;
class VistaConnectionIP;
class CITANetAudioMessage;
class CITANetAudioProtocol;

class CITANetAudioStreamingClient : public VistaThreadLoop
{
public:

	CITANetAudioStreamingClient( CITANetAudioStream* pParent );
	~CITANetAudioStreamingClient();

	bool Connect( const std::string& sAddress, int iPort );
	void Disconnect();
	bool LoopBody();
	bool GetIsConnected();

private:
	CITANetAudioStream* m_pParent;

	VistaConnectionIP* m_pConnection;
	CITANetAudioProtocol* m_pProtocol;
	CITANetAudioMessage* m_pMessage;

	ITASampleFrame m_sfReceivingBuffer;
	bool m_bStopIndicated;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_CONNECTION
