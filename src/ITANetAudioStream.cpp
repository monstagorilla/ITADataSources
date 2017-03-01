#include <ITANetAudioStream.h>

#include <ITANetAudioStreamingClient.h>

// ITA includes
#include <ITAException.h>
#include <ITADataLog.h>
#include <ITAStreamInfo.h>
#include <ITAClock.h>
#include <iomanip> 

// Vista includes
#include <VistaBase/VistaStreamUtils.h>

// STL includes
#include <cmath>
#include <iostream>

//! Audio streaming log item
struct ITAAudioLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "\t" << "Channel";
		os << "\t" << "Samplerate";
		os << "\t" << "BufferSize";
		os << "\t" << "RingBufferSize";
		os << "\t" << "TargetSampleLatency";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << "\t" << iChannel;
		os << "\t" << dSampleRate;
		os << "\t" << iBufferSize;
		os << "\t" << iRingBufferSize;
		os << "\t" << iTargetSampleLatency;
		os << std::endl;
		return os;
	};

	int iChannel;
	double dSampleRate;
	int iBufferSize;
	int iRingBufferSize;
	int iTargetSampleLatency;
};
	//! Audio streaming log item
struct ITAStreamLog : public ITALogDataBase
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
		os << "\t" << iStreamingStatus;
		os << "\t" << iFreeSamples;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	double dStreamingTimeCode;
	int iStreamingStatus; //!< ... usw
	int iFreeSamples;

};

//! Network streaming log item
struct ITANetLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "Bufferstatus";
		os << "\t" << "FreeSamples";
		os << "\t" << "NumSamplesTransmitted";
		os << std::endl;
		return os;
	};
	
	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << iBufferStatus;
		os << "\t" << iFreeSamples;
		os << "\t" << iNumSamplesTransmitted;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId;
	double dWorldTimeStamp;
	int iBufferStatus;
	int iFreeSamples;
	int iNumSamplesTransmitted;
};

class ITABufferedDataLoggerImplAudio : public ITABufferedDataLogger < ITAAudioLog > {};
class ITABufferedDataLoggerImplStream : public ITABufferedDataLogger < ITAStreamLog > {};
class ITABufferedDataLoggerImplNet : public ITABufferedDataLogger < ITANetLog > {};


CITANetAudioStream::CITANetAudioStream(int iChannels, double dSamplingRate, int iBufferSize, int iTargetSampleLatencyServer)
	: m_sfOutputStreamBuffer( iChannels, iBufferSize, true )
	, m_dSampleRate( dSamplingRate )
	, m_sfRingBuffer(iChannels, iTargetSampleLatencyServer * 3, true)
	, m_bRingBufferFull( false )
	, m_iStreamingStatus( INVALID )
	, m_dLastStreamingTimeCode( 0.0f )
	, m_iTargetSampleLatencyServer( iTargetSampleLatencyServer )
{
	m_bRingBufferFull = false;
	if (iBufferSize > iTargetSampleLatencyServer)
		ITA_EXCEPT1( INVALID_PARAMETER, "Ring buffer capacity can not be smaller than Target Sample Latency." );

	m_pNetAudioStreamingClient = new CITANetAudioStreamingClient( this );
	m_iReadCursor = 0;
	m_iWriteCursor = 0; // always ahead, i.e. iWriteCursor >= iReadCursor if unwrapped

	m_iStreamingStatus = STOPPED;

	// Logging
	std::string paras = std::string("NetAudioLogBaseData") + std::string("_BS") + std::to_string(iBufferSize) + std::string("_Ch") + std::to_string(iChannels) + std::string(".txt");
	m_pAudioLogger = new ITABufferedDataLoggerImplAudio( );
	m_pAudioLogger->setOutputFile(paras);

	paras = std::string("NetAudioLogStream") + std::string("_BS") + std::to_string(iBufferSize) + std::string("_Ch") + std::to_string(iChannels) + std::string(".txt");
	m_pStreamLogger = new ITABufferedDataLoggerImplStream();
	m_pStreamLogger->setOutputFile(paras);
	iAudioStreamingBlockID = 0;

	paras = std::string("NetAudioLogNet") + std::string("_BS") + std::to_string(iBufferSize) + std::string("_Ch") + std::to_string(iChannels) + std::string(".txt");
	m_pNetLogger = new ITABufferedDataLoggerImplNet();
	m_pNetLogger->setOutputFile(paras);
	iNetStreamingBlockID = 0;

	// Logging Base Data
	ITAAudioLog oLog;
	oLog.iChannel = GetNumberOfChannels();
	oLog.dSampleRate = m_dSampleRate;
	oLog.iBufferSize = GetBlocklength();
	oLog.iRingBufferSize = GetRingBufferSize();
	oLog.iTargetSampleLatency = m_iTargetSampleLatencyServer;
	m_pAudioLogger->log( oLog );
}

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetLogger;
	delete m_pStreamLogger;
	delete m_pAudioLogger;
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

void CITANetAudioStream::SetAllowedLatencySeconds( float fLatencySeconds )
{
	SetAllowedLatencySamples( (int) std::floor( GetSampleRate() * fLatencySeconds ) );
}

float CITANetAudioStream::GetMaximumLatencySeconds() const
{
	return float( GetMaximumLatencySamples() / GetSampleRate() );
}

float CITANetAudioStream::GetMinimumLatencySeconds() const
{
	return float( GetMinimumLatencySamples() / GetSampleRate() );
}

void CITANetAudioStream::SetAllowedLatencySamples( int iLatencySamples )
{
	if( iLatencySamples < GetMinimumLatencySamples() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Can not set latency lower than the minimum possible" );

	if( iLatencySamples > GetMaximumLatencySamples() )
		ITA_EXCEPT1( INVALID_PARAMETER, "Can not set latency greater than the maximum possible" );

	m_iTargetSampleLatencyServer = iLatencySamples;
}

float CITANetAudioStream::GetAllowedLatencySeconds() const
{
	return float(m_iTargetSampleLatencyServer / GetSampleRate());
}

int CITANetAudioStream::GetAllowedLatencySamples() const
{
	return m_iTargetSampleLatencyServer;
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

void CITANetAudioStream::SetLatencyForRealtime()
{
	SetAllowedLatencySamples( GetMinimumLatencySamples() );
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

	ITAStreamLog oLog;	
	oLog.iStreamingStatus = m_iStreamingStatus;
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock()->getTime();
	oLog.dStreamingTimeCode = m_dLastStreamingTimeCode;
	oLog.uiBlockId = ++iAudioStreamingBlockID;
	oLog.iFreeSamples = GetRingBufferFreeSamples( );
	m_pStreamLogger->log( oLog );
	
	//m_pNetAudioStreamingClient->TriggerBlockIncrement();
}

int CITANetAudioStream::Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples )
{
	// Take local copies (concurrent access)
	int iCurrentReadCursor = m_iReadCursor;
	int iCurrentWriteCursor = m_iWriteCursor;

	ITANetLog oLog;
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
	oLog.iBufferStatus = m_iStreamingStatus;
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
	oLog.uiBlockId = ++iAudioStreamingBlockID;
	oLog.iFreeSamples = GetRingBufferFreeSamples( );
	oLog.iNumSamplesTransmitted = iNumSamples;
	m_pNetLogger->log( oLog );
	
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

	int iFreeSamples = GetRingBufferSize() - ((m_iWriteCursor - m_iReadCursor + GetRingBufferSize()) % GetRingBufferSize());
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
