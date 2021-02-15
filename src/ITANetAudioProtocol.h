/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2021
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
	static const int NP_CLIENT_IDLE = 0;
	static const int NP_CLIENT_OPEN = 100;
	static const int NP_CLIENT_CLOSE = 101;
	static const int NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES = 111;

	static const int NP_SERVER_OPEN = 200;
	static const int NP_SERVER_CLOSE = 201;
	static const int NP_SERVER_REFUSED_INVALID_PARAMETERS = 202;
	static const int NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES = 211;
	static const int NP_SERVER_SENDING_SAMPLES = 222;

	inline CITANetAudioProtocol() {};
	inline ~CITANetAudioProtocol() {};

	inline static std::string GetNPMessageID( const int iMessageType )
	{
		switch( iMessageType )
		{
		case NP_CLIENT_IDLE: return "CLIENT_IDLE";
		case NP_CLIENT_OPEN: return "CLIENT_OPEN";
		case NP_CLIENT_CLOSE: return "CLIENT_CLOSE";
		case NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES: return "CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES";
		case NP_SERVER_OPEN: return "SERVER_OPEN";
		case NP_SERVER_REFUSED_INVALID_PARAMETERS: return "NP_SERVER_REFUSED_INVALID_PARAMETERS";
		case NP_SERVER_CLOSE: return "SERVER_CLOSE";
		case NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES: return "SERVER_GET_RINGBUFFER_FREE_SAMPLES";
		case NP_SERVER_SENDING_SAMPLES: return "SERVER_SENDING_SAMPLES";
		default: return "INVALID";
		}
	};
	
	struct StreamingParameters
	{
		int iChannels;
		double dSampleRate;
		int iBlockSize;
		int iRingBufferSize;

		inline StreamingParameters()
		{
			iChannels = 0;
			dSampleRate = 0.0f;
			iBlockSize = 0;
			iRingBufferSize = 0;
		};

		inline bool operator==( const StreamingParameters& rhs )
		{
			if ( ( iChannels == rhs.iChannels ) 
				&& ( dSampleRate == rhs.dSampleRate ) 
				&& (iBlockSize == rhs.iBlockSize)
				&& (iRingBufferSize == rhs.iRingBufferSize))
				return true;
			else
				return false;
		};
	};
};

#endif // INCLUDE_WATCHER_ITA_NET_AUDIO_PROTOCOL
