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

#ifndef INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL
#define INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL

#include <ITADataSourcesDefinitions.h>

// ITA includes
#include <ITAException.h>
#include <ITASampleBuffer.h>
#include <ITASampleFrame.h>

// Vista includes
#include <VistaBase/VistaBaseTypes.h>
#include <VistaInterProcComm/Connections/VistaByteBufferSerializer.h>
#include <VistaInterProcComm/Connections/VistaByteBufferDeSerializer.h>

// STL includes
#include <string>
#include <vector>

// Forward declarations
class VistaConnectionIP;
class CITANetAudioStreamingServer;
class CITANetAudioStream;

//! Network audio protocol
/**
  * Declaration of message types
  */
class ITA_DATA_SOURCES_API CITANetAudioProtocol
{
public:
	static const int NET_AUDIO_VERSION = 1;

	static const int NP_INVALID = -1;
	static const int NP_GET_VERSION_INFO = 1;

	static const int NP_CLIENT_OPEN = 100;
	static const int NP_CLIENT_CLOSE = 101;

	static const int NP_SERVER_CLOSE = 200;
	static const int NP_SERVER_OPEN = 201;
	static const int NP_SERVER_GET_RINGBUFFER_SIZE = 210;
	static const int NP_SERVER_GET_RINGBUFFER_FREE = 211;
	static const int NP_SERVER_SEND_SAMPLES = 222;

	CITANetAudioProtocol();
	virtual ~CITANetAudioProtocol();
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL
