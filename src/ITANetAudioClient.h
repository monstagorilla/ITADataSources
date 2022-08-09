/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2022
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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_CLIENT
#define INCLUDE_WATCHER_ITA_NET_AUDIO_CLIENT

#include <ITADataSourcesDefinitions.h>
#include <string>

class VistaConnectionIP;

//! A network audio client that connects to a network audio server
/**
 * Use CITANetAudioStreamingClient to start an audio stream with the connection of this client.
 * This class is basically a helper around Vista TCP/IP or UDP network functionality.
 *
 */
class CITANetAudioClient
{
public:
	//! Create an network audio client that connects to a network audio server
	/**
	 * \param pParent ITADataSource-compatible audio stream
	 */
	CITANetAudioClient( );
	~CITANetAudioClient( );

	bool Connect( const std::string& sAddress, const int iPort, const bool bUseUDP );
	void Disconnect( );
	bool GetIsConnected( ) const;

	VistaConnectionIP* GetConnection( ) const;

private:
	VistaConnectionIP* m_pConnection;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_CLIENT
