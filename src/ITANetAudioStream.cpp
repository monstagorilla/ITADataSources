#include <ITANetAudioStream.h>

#include <ITANetAudioStreamingClient.h>

// ITA includes
#include <ITAException.h>

// STL
#include <cmath>

CITANetAudioStream::CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity )
	: m_sfOutputStreamBuffer( iChannels, iBufferSize, true )
	, m_dSampleRate( dSamplingRate )
	, m_sfRingBuffer( iChannels, iRingBufferCapacity, true )
	, m_dSampleRate( dSamplingRate )
	
{
	if( iBufferSize > iRingBufferCapacity )
		ITA_EXCEPT1( INVALID_PARAMETER, "Ring buffer capacity can not be smaller than buffer size." );

	m_pNetAudioStreamingClient = new CITANetAudioStreamingClient( this );
}

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetAudioStreamingClient;
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	return m_pNetAudioStreamingClient->Connect( sAddress, iPort );
}

bool CITANetAudioStream::GetIsConnected() const
{
	return m_pNetAudioStreamingClient->GetIsConnected();
}

const float* CITANetAudioStream::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* )
{
	// @todo: implement cyclic read from ring buffer
	
	return m_sfOutputStreamBuffer[ uiChannel ].GetData();
}

void CITANetAudioStream::IncrementBlockPointer()
{
	// Increment read cursor by one audio block and wrap around if exceeding ring buffer
	m_iReadCursor = ( m_iReadCursor + m_sfOutputStreamBuffer.GetLength() ) % m_sfRingBuffer.GetLength();
}

int CITANetAudioStream::Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples )
{
	ITA_EXCEPT0( NOT_IMPLEMENTED );
}

int CITANetAudioStream::GetRingbufferFreeSamples()
{
	ITA_EXCEPT0( NOT_IMPLEMENTED );
}

int CITANetAudioStream::GetRingBufferSize() const
{
	return m_sfRingBuffer.GetLength();
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
