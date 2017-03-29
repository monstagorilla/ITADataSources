#include <ITANetAudioStream.h>

#include "ITANetAudioStreamingClient.h"

// ITA includes
#include <ITAException.h>
#include <ITADataLog.h>
#include <ITAStreamInfo.h>
#include <ITAClock.h>

// Vista includes
#include <VistaBase/VistaStreamUtils.h>

// STL includes
#include <cmath>
#include <iomanip>
#include <iostream>


//! Audio streaming log item
struct ITAAudioStreamLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "StreamingTimeCode";
		os << "\t" << "StreamingStatus";
		os << "\t" << "FreeSamples";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << std::setprecision( 12 ) << dStreamingTimeCode;
		os << "\t" << sStreamingStatus;
		os << "\t" << iFreeSamples;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	double dStreamingTimeCode;
	std::string sStreamingStatus;
	int iFreeSamples;

};

//! Network streaming log item
struct ITANetworkStreamLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "BufferStatus";
		os << "\t" << "FreeSamples";
		os << "\t" << "NumSamplesTransmitted";
		os << std::endl;
		return os;
	};
	
	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << sBufferStatus;
		os << "\t" << iFreeSamples;
		os << "\t" << iNumSamplesTransmitted;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId;
	double dWorldTimeStamp;
	std::string sBufferStatus;
	int iFreeSamples;
	int iNumSamplesTransmitted;
};

class ITABufferedDataLoggerImplStream : public ITABufferedDataLogger < ITAAudioStreamLog > {};
class ITABufferedDataLoggerImplNet : public ITABufferedDataLogger < ITANetworkStreamLog > {};


CITANetAudioStream::CITANetAudioStream(int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity /* = 2048 */)
	: m_sfOutputStreamBuffer( iChannels, iBufferSize, true )
	, m_dSampleRate( dSamplingRate )
	, m_sfRingBuffer( iChannels, iRingBufferCapacity, true )
	, m_bRingBufferFull( false )
	, m_iStreamingStatus( INVALID )
	, m_dLastStreamingTimeCode( 0.0f )
	, m_iReadCursor( 0 )
	, m_iWriteCursor( 0 ) // always ahead, i.e. iWriteCursor >= iReadCursor if unwrapped
	, m_iAudioStreamingBlockID( 0 )
	, m_iNetStreamingBlockID( 0 )
	, m_bDebuggingEnabled( false )
{
	if( iBufferSize > iRingBufferCapacity )
		ITA_EXCEPT1( INVALID_PARAMETER, "Ring buffer capacity can not be smaller than Target Sample Latency." );

	m_pNetAudioStreamingClient = new CITANetAudioStreamingClient( this );

	// Logging
	m_pAudioStreamLogger = new ITABufferedDataLoggerImplStream();
	m_pNetworkStreamLogger = new ITABufferedDataLoggerImplNet();

	SetNetAudioStreamingLoggerBaseName( "ITANetAudioStream" );

	 m_iStreamingStatus = STOPPED;
}

CITANetAudioStream::~CITANetAudioStream()
{
	if( m_bDebuggingEnabled == false )
	{
		m_pAudioStreamLogger->setOutputFile( "" ); // Disables file storing
		m_pNetworkStreamLogger->setOutputFile( "" ); // Disables file storing
	}

	delete m_pAudioStreamLogger;
	delete m_pNetworkStreamLogger;
	delete m_pNetAudioStreamingClient;
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	return m_pNetAudioStreamingClient->Connect( sAddress, iPort );
}

void CITANetAudioStream::Disconnect()
{
	m_pNetAudioStreamingClient->Disconnect();
}

bool CITANetAudioStream::GetIsConnected() const
{
	return m_pNetAudioStreamingClient->GetIsConnected();
}

float CITANetAudioStream::GetMaximumLatencySeconds() const
{
	return float( GetMaximumLatencySamples() / GetSampleRate() );
}

float CITANetAudioStream::GetMinimumLatencySeconds() const
{
	return float( GetMinimumLatencySamples() / GetSampleRate() );
}

int CITANetAudioStream::GetMinimumLatencySamples() const
{
	// At least one block
	return GetBlocklength();
}

int CITANetAudioStream::GetMaximumLatencySamples() const
{
	return GetRingBufferSize();
}

const float* CITANetAudioStream::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pInfo )
{
	if( !GetIsConnected() )
	{
		m_sfOutputStreamBuffer[ uiChannel ].Zero( );
		if (uiChannel == 0 )
			m_iStreamingStatus = STOPPED;
	}
	else
	{
		if( GetIsRingBufferEmpty() )
		{
			m_sfOutputStreamBuffer[ uiChannel ].Zero();
			if (uiChannel == 0 )
				m_iStreamingStatus = BUFFER_UNDERRUN;
#if NET_AUDIO_SHOW_TRAFFIC
			//vstr::out() << "[ Stream ] Buffer underrun" << std::endl;
#endif
		}
		else
		{
			if( GetRingBufferAvailableSamples() < int( GetBlocklength() ) )
			{
				// @todo: fade out
				m_sfRingBuffer[ uiChannel ].Zero();
				if (uiChannel == 0 )
					m_iStreamingStatus = BUFFER_UNDERRUN;
#if NET_AUDIO_SHOW_TRAFFIC
				//vstr::out() << "[ Stream ] Buffer underrun" << std::endl;
#endif
			}
			else
			{
				// Normal behaviour (if everything is OK with ring buffer status)
				m_sfRingBuffer[ uiChannel ].cyclic_read( m_sfOutputStreamBuffer[ uiChannel ].GetData(), GetBlocklength(), m_iReadCursor );
				if ( uiChannel == 0 )
					m_iStreamingStatus = STREAMING;
#if NET_AUDIO_SHOW_TRAFFIC
				vstr::out() << "[ Stream ] Streaming" << std::endl;
#endif
			}
		}		
	}

	if( uiChannel == 0 )
		m_dLastStreamingTimeCode = pInfo->dTimecode;


	return m_sfOutputStreamBuffer[uiChannel].GetData();
}

void CITANetAudioStream::IncrementBlockPointer()
{
	// Increment read cursor by one audio block and wrap around if exceeding ring buffer
	int iSavedSample = GetRingBufferSize( ) - GetRingBufferFreeSamples( );
	if ( !GetIsConnected( ) )
	{
		//m_iStreamingStatus = STOPPED;
	} else if ( iSavedSample >= int( GetBlocklength( ) ) )
	{
		//es wurden Samples abgespielt
		m_iReadCursor = ( m_iReadCursor + m_sfOutputStreamBuffer.GetLength() ) % m_sfRingBuffer.GetLength();
		//m_iStreamingStatus = STREAMING;
#if NET_AUDIO_SHOW_TRAFFIC
		//vstr::out() << "[ Stream ] Streaming" << std::endl;
#endif
	}
	else if ( GetIsRingBufferEmpty( ) )
	{
		//m_iStreamingStatus = BUFFER_UNDERRUN;
#if NET_AUDIO_SHOW_TRAFFIC
		//vstr::out() << "[ Stream ] Buffer underrun" << std::endl;
#endif
	}
	else
	{
		//m_iStreamingStatus = BUFFER_UNDERRUN;
#if NET_AUDIO_SHOW_TRAFFIC
		//vstr::out() << "[ Stream ] Buffer underrun" << std::endl;
#endif
		m_iReadCursor = m_iWriteCursor;
	}
	m_bRingBufferFull = false;

	ITAAudioStreamLog oLog;	
	oLog.sStreamingStatus = GetStreamingStatusString( m_iStreamingStatus );
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock()->getTime();
	oLog.dStreamingTimeCode = m_dLastStreamingTimeCode;
	oLog.uiBlockId = ++m_iAudioStreamingBlockID;
	oLog.iFreeSamples = GetRingBufferFreeSamples( );
	m_pAudioStreamLogger->log( oLog );
}

int CITANetAudioStream::Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples )
{
	// Take local copies (concurrent access)
	int iCurrentReadCursor = m_iReadCursor;
	int iCurrentWriteCursor = m_iWriteCursor;

	if( iCurrentWriteCursor < iCurrentReadCursor )
		iCurrentWriteCursor += GetRingBufferSize(); // Unwrap, because write cursor always ahead

	if ( ( m_iWriteCursor == m_iReadCursor ) && m_bRingBufferFull )
	{
		// BufferFull
		m_iStreamingStatus = BUFFER_OVERRUN;
#if NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ NetAudio ] Buffer overrun" << std::endl;
#endif
	}
	else if( GetRingBufferFreeSamples() < iNumSamples )
	{
		// @todo: only partly write
		//std::cerr << "BUFFER_OVERRUN! Would partly write samples because ring buffer will be full then." << std::endl;

		m_iStreamingStatus = BUFFER_OVERRUN;
		m_iWriteCursor = m_iReadCursor;
	}
	else
	{
		// write samples into ring buffer
		m_sfRingBuffer.cyclic_write( sfNewSamples, iNumSamples, 0, iCurrentWriteCursor );
		m_bRingBufferFull = false;
		m_iStreamingStatus = STREAMING;
#if NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ NetAudio ] Buffer write" << std::endl;
#endif

		// set write curser
		m_iWriteCursor = ( m_iWriteCursor + iNumSamples ) % GetRingBufferSize( );
		if ( m_iWriteCursor == m_iReadCursor )
		{
			m_bRingBufferFull = true;
#if NET_AUDIO_SHOW_TRAFFIC
			vstr::out() << "[ NetAudio ] Buffer overrun" << std::endl;
#endif
		}
	}

	ITANetworkStreamLog oLog;
	oLog.sBufferStatus = GetStreamingStatusString( m_iStreamingStatus );
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
	oLog.uiBlockId = ++m_iAudioStreamingBlockID;
	oLog.iFreeSamples = GetRingBufferFreeSamples();
	oLog.iNumSamplesTransmitted = iNumSamples;
	m_pNetworkStreamLogger->log( oLog );
	
	return GetRingBufferFreeSamples();
}

int CITANetAudioStream::GetRingBufferAvailableSamples() const
{
	return GetRingBufferSize() - GetRingBufferFreeSamples();
}

int CITANetAudioStream::GetRingBufferFreeSamples() const
{
	if( m_bRingBufferFull )
		return 0;

	int iFreeSamples = GetRingBufferSize() - ( ( m_iWriteCursor - m_iReadCursor + GetRingBufferSize() ) % GetRingBufferSize() );
	assert( iFreeSamples > 0 );
	return iFreeSamples;
}

int CITANetAudioStream::GetRingBufferSize() const
{
	return m_sfRingBuffer.GetLength();
}

bool CITANetAudioStream::GetIsRingBufferFull() const
{
	return m_bRingBufferFull;
}

bool CITANetAudioStream::GetIsRingBufferEmpty() const
{
	return ( !m_bRingBufferFull && m_iReadCursor == m_iWriteCursor );
}

unsigned int CITANetAudioStream::GetBlocklength() const
{
	return  ( unsigned int ) m_sfOutputStreamBuffer.GetLength();
}

unsigned int CITANetAudioStream::GetNumberOfChannels() const
{
	return ( unsigned int ) m_sfOutputStreamBuffer.channels();
}

double CITANetAudioStream::GetSampleRate() const
{
	return m_dSampleRate;
}

std::string CITANetAudioStream::GetStreamingStatusString( int iStreamingStatus )
{
	if( iStreamingStatus == CITANetAudioStream::INVALID )
		return "INVALID";
	if( iStreamingStatus == CITANetAudioStream::STOPPED )
		return "STOPPED";
	if( iStreamingStatus == CITANetAudioStream::CONNECTED )
		return "CONNECTED";
	if( iStreamingStatus == CITANetAudioStream::STREAMING )
		return "STREAMING";
	if( iStreamingStatus == CITANetAudioStream::BUFFER_UNDERRUN )
		return "BUFFER_UNDERRUN";
	if( iStreamingStatus == CITANetAudioStream::BUFFER_OVERRUN )
		return "BUFFER_OVERRUN";

	return "UNKOWN";
}

std::string CITANetAudioStream::GetNetAudioStreamLoggerBaseName() const
{
	return m_sNetAudioStreamLoggerBaseName;
}

void CITANetAudioStream::SetNetAudioStreamingLoggerBaseName( const std::string& sBaseName )
{
	m_sNetAudioStreamLoggerBaseName = sBaseName;
	m_pAudioStreamLogger->setOutputFile( GetNetAudioStreamLoggerBaseName() + "_AudioStream.log" );
	m_pNetworkStreamLogger->setOutputFile( GetNetAudioStreamLoggerBaseName() + "_NetworkStream.log" );
	m_pNetAudioStreamingClient->SetClientLoggerBaseName( sBaseName );
}

void CITANetAudioStream::SetDebuggingEnabled( bool bEnabled )
{
	m_bDebuggingEnabled = bEnabled;
}

bool CITANetAudioStream::GetIsDebuggingEnabled() const
{
	return m_bDebuggingEnabled;
}
