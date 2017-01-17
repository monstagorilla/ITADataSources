#include <ITANetAudioStream.h>

#include <ITANetAudioStreamingClient.h>

// ITA includes
#include <ITAException.h>
#include <ITADataLog.h>
#include <ITAStreamInfo.h>
#include <ITAClock.h>
#include <iomanip> 

// STL
#include <cmath>
#include <iostream>


//! Audio streaming log item
struct ITAStreamLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "TimeCode";
		os << "\t" << "StreamingStatus";
		os << "\t" << "FreeSamples";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dTimecode;
		os << "\t" << iStreamingStatus;
		os << "\t" << iFreeSamples;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	int iStreamingStatus; //!< ... usw
	double dTimecode;
	int iFreeSamples;

};

//! Network streaming log item
struct ITANetLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId " << "\tTimeStamp" << "\tBufferstatus" << std::endl;
		return os;
	};
	
	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << "NetLog\t"<< uiBlockId << "\t" << std::setprecision( 12 ) << dTimecode << "\t" << iBufferStatus << "\t" << iFreeSamples << std::endl;
		return os;
	};

	unsigned int uiBlockId;
	int iBufferStatus;
	double dTimecode;
	int iFreeSamples;

};

class ITABufferedDataLoggerImplStream : public ITABufferedDataLogger < ITAStreamLog > {};
class ITABufferedDataLoggerImplNet : public ITABufferedDataLogger < ITANetLog > {};


CITANetAudioStream::CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity )
	: m_sfOutputStreamBuffer( iChannels, iBufferSize, true )
	, m_dSampleRate( dSamplingRate )
	, m_sfRingBuffer( iChannels, iRingBufferCapacity, true )
	, m_bRingBufferFull( false )
	, m_iStreamingStatus( INVALID )
	
{
	m_bRingBufferFull = false;
	if( iBufferSize > iRingBufferCapacity )
		ITA_EXCEPT1( INVALID_PARAMETER, "Ring buffer capacity can not be smaller than buffer size." );

	m_pNetAudioStreamingClient = new CITANetAudioStreamingClient( this );
	m_iReadCursor = 0;
	m_iWriteCursor = 0; // always ahead, i.e. iWriteCursor >= iReadCursor if unwrapped

	m_iStreamingStatus = STOPPED;

	// Logging
	m_pStreamLogger = new ITABufferedDataLoggerImplStream();
	m_pStreamLogger->setOutputFile( "NetAudioLogStream.txt" );
	iAudioStreamingBlockID = 0;

	m_pNetLogger = new ITABufferedDataLoggerImplNet();
	m_pNetLogger->setOutputFile( "NetAudioLogNet.txt" );
	iNetStreamingBlockID = 0;
}

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetAudioStreamingClient; 
	delete m_pStreamLogger;
	delete m_pNetLogger;
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	return m_pNetAudioStreamingClient->Connect( sAddress, iPort );
}

bool CITANetAudioStream::GetIsConnected() const
{
	return m_pNetAudioStreamingClient->GetIsConnected();
}

const float* CITANetAudioStream::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pInfo )
{
	if( !GetIsConnected() )
	{
		m_sfOutputStreamBuffer[ uiChannel ].Zero( );
	}
	else
	{
		if( GetIsRingBufferEmpty() )
		{
			m_sfOutputStreamBuffer[ uiChannel ].Zero();
			m_iStreamingStatus = BUFFER_UNDERRUN;
		}
		else
		{
			if( GetRingBufferAvailableSamples() < GetBlocklength() )
			{
				// @todo: fade out
				m_sfRingBuffer[ uiChannel ].Zero();
				m_iStreamingStatus = BUFFER_UNDERRUN;
			}
			else
			{
				// Normal behaviour (if everything is OK with ring buffer status)
				m_sfRingBuffer[ uiChannel ].cyclic_read( m_sfOutputStreamBuffer[ uiChannel ].GetData(), GetBlocklength(), m_iReadCursor );
				m_iStreamingStatus = STREAMING;
			}
		}		
	}

	if ( uiChannel == 0 )
	{
		ITAStreamLog oLog;
		oLog.iStreamingStatus = m_iStreamingStatus;
		oLog.dTimecode = ITAClock::getDefaultClock( )->getTime( );
		oLog.uiBlockId = ++iAudioStreamingBlockID;
		oLog.iFreeSamples = GetRingBufferFreeSamples( );
		m_pStreamLogger->log( oLog );
	}

	return m_sfOutputStreamBuffer[uiChannel].GetData();
}

void CITANetAudioStream::IncrementBlockPointer()
{
	// Increment read cursor by one audio block and wrap around if exceeding ring buffer
	int iSavedSample = GetRingBufferSize( ) - GetRingBufferFreeSamples( );

	if ( iSavedSample >= int( GetBlocklength( ) ) )
	{
		//es wurden Samples abgespielt
		m_iReadCursor = ( m_iReadCursor + m_sfOutputStreamBuffer.GetLength() ) % m_sfRingBuffer.GetLength();
		m_iStreamingStatus = STREAMING;
	}
	else if ( GetIsRingBufferEmpty( ) )
	{
		m_iStreamingStatus = BUFFER_UNDERRUN;
	}
	else
	{
		m_iStreamingStatus = BUFFER_OVERRUN;
		m_iReadCursor = m_iWriteCursor;
	}
	m_bRingBufferFull = false;
	
	m_pNetAudioStreamingClient->TriggerBlockIncrement();
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
		oLog.iBufferStatus = 1;
	}
	else if( GetRingBufferFreeSamples() < iNumSamples )
	{
		// @todo: only partly write
		//std::cerr << "BUFFER_OVERRUN! Would partly write samples because ring buffer will be full then." << std::endl;
		
		m_iWriteCursor = m_iReadCursor;
		oLog.iBufferStatus = 2;
	}
	else
	{
		// write samples into ring buffer
		m_sfRingBuffer.cyclic_write( sfNewSamples, iNumSamples, 0, iCurrentWriteCursor );
		m_bRingBufferFull = false;
		oLog.iBufferStatus = 1;

		// set write curser
		m_iWriteCursor = ( m_iWriteCursor + iNumSamples ) % GetRingBufferSize( );
		if ( m_iWriteCursor == m_iReadCursor )
		{
			m_bRingBufferFull = true;
			oLog.iBufferStatus = 1;
		}
	}

	oLog.dTimecode = ITAClock::getDefaultClock( )->getTime( );
	iAudioStreamingBlockID++;
	oLog.uiBlockId = iAudioStreamingBlockID;
	oLog.iFreeSamples = GetRingBufferFreeSamples( );
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
