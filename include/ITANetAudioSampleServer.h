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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER
#define INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER

#include <ITADataSourcesDefinitions.h>

#include <string>
#include <vector>

class ITADatasource;

//! Network audio sample server (for connecting a net audio stream)
/**
 * Audio sample transmitter for a networked signal source that can connect via TCP/IP.
 *
 * \sa CITANetAudioStream
 * \note not thread-safe
 */
class CITANetAudioSampleServer
{
public:
	CITANetAudioSampleServer();
	virtual ~CITANetAudioStream();

	bool Start( const std::string& sAddress, int iPort );
	bool IsClientConnected() const;
	std::string GetNetworkAddress() const;
	int GetNetworkPort() const;
	int Stop();

	void SetInputStream( ITADatasource* pInStream );
	
	int GetNetStreamBlocklength() const;
	int GetNetStreamNumberOfChannels() const;
	double GetNetStreamSampleRate() const;

protected:
	int Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples );

private:
	CITANetAudioStreamServer* m_pNetAudioServer;
	ITASampleFrame m_sfTempTransmitBuffer;
	ITADatasource* m_pInputStream;

	friend class CITANetAudioStreamServer;
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_SAMPLE_SERVER
