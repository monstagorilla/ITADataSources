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
struct ITAStreamLog : public ITALogDataBase {

	virtual std::ostream& outputDesc( std::ostream& os ) const {
		os << "BlockId " << "\tTimeStamp" << "\tStreamingStatus" << std::endl;
		return os;
	};
	virtual std::ostream& outputData( std::ostream& os ) const {
		os << "StreamLog\t" << uiBlockId << "\t" << std::setprecision( 12 ) << dTimecode << "\t" << iStreamingStatus << "\t" << iFreeSamples << std::endl;
		return os;
	};

	unsigned int uiBlockId;
	int iStreamingStatus;
	double dTimecode;
	int iFreeSamples;

};
struct ITANetLog : public ITALogDataBase {

	virtual std::ostream& outputDesc( std::ostream& os ) const {
		os << "BlockId " << "\tTimeStamp" << "\tBufferstatus" << std::endl;
		return os;
	};
	virtual std::ostream& outputData( std::ostream& os ) const {
		os << "NetLog\t"<< uiBlockId << "\t" << std::setprecision( 12 ) << dTimecode << "\t" << iBufferStatus << "\t" << iFreeSamples << std::endl;
		return os;
	};

	unsigned int uiBlockId;
	int iBufferStatus;
	double dTimecode;
	int iFreeSamples;

};
class ITABufferedDataLoggerImplStream : public ITABufferedDataLogger<ITAStreamLog> {
public:
};
class ITABufferedDataLoggerImplNet : public ITABufferedDataLogger<ITANetLog> {
public:
};

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
	m_pStreamLogger = new ITABufferedDataLoggerImplStream( );
	m_pStreamLogger->setOutputFile( "NetAudioStream.log" );
	m_pNetLogger = new ITABufferedDataLoggerImplNet( );
	m_pNetLogger->setOutputFile( "NetAudioNet.log" );
	iID = 0;
}

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetAudioStreamingClient; 
	delete m_pStreamLogger;
	delete m_pNetLogger;
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	bool bConnected = m_pNetAudioStreamingClient->Connect( sAddress, iPort );
	if( bConnected )
		m_iStreamingStatus = CONNECTED;
	return bConnected;
}

bool CITANetAudioStream::GetIsConnected() const
{
	return m_pNetAudioStreamingClient->GetIsConnected();
}

const float* CITANetAudioStream::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pInfo )
{
	if ( !GetIsConnected( ) )
	{
		m_sfOutputStreamBuffer[ uiChannel ].Zero( );
		return m_sfOutputStreamBuffer[ uiChannel ].GetData( );
	}

	if ( GetIsRingBufferEmpty( ) )
	{
		m_sfOutputStreamBuffer[ uiChannel ].Zero( );
	}
	// Es ist mindestens ein Block da
	else
	{
		// Es ist mindestens ein Block da
		m_sfRingBuffer[ uiChannel ].cyclic_read( m_sfOutputStreamBuffer[ uiChannel ].GetData( ), m_sfOutputStreamBuffer.GetLength( ), m_iReadCursor );
		// weniger als ein Block
			// todo fading
		
	}
	if ( uiChannel == 0 )
	{
		ITAStreamLog oLog;
		oLog.iStreamingStatus = m_iStreamingStatus;
		oLog.dTimecode = ITAClock::getDefaultClock( )->getTime( );
		iID++;
		oLog.uiBlockId = iID;
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
		m_iStreamingStatus = STOPPED;
	}
	else
	{
		m_iStreamingStatus = BUFFER_UNDERRUN;
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
	iID++;
	oLog.uiBlockId = iID;
	oLog.iFreeSamples = GetRingBufferFreeSamples( );
	m_pNetLogger->log( oLog );
	
	return GetRingBufferFreeSamples();
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
