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
	
{
	if( iBufferSize > iRingBufferCapacity )
		ITA_EXCEPT1( INVALID_PARAMETER, "Ring buffer capacity can not be smaller than buffer size." );

	m_pNetAudioStreamingClient = new CITANetAudioStreamingClient( this );
	m_iReadCursor = 0;
	m_iWriteCursor = 0;
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
	// @todo: is connected?
	int iCurrentWritePointer = m_iWriteCursor;
	if (iCurrentWritePointer > m_iReadCursor) {
		m_sfOutputStreamBuffer[uiChannel].cyclic_write(&m_sfRingBuffer[uiChannel], 
			m_sfOutputStreamBuffer.GetLength(), m_iReadCursor, iCurrentWritePointer);
	} else {
		// in diesem Block alle Kanaele auf 0 setzen
		m_sfOutputStreamBuffer[uiChannel].Zero();
	}
	
	return m_sfOutputStreamBuffer[ uiChannel ].GetData();
}

void CITANetAudioStream::IncrementBlockPointer()
{
	// Increment read cursor by one audio block and wrap around if exceeding ring buffer
	m_iReadCursor = ( m_iReadCursor + m_sfOutputStreamBuffer.GetLength() ) % m_sfRingBuffer.GetLength();
	m_pNetAudioStreamingClient->TriggerBlockIncrement();
}

int CITANetAudioStream::Transmit( const ITASampleFrame& sfNewSamples, int iNumSamples )
{
	int iCurrentReadCursor = m_iReadCursor;

	//kopiert Samples in den RingBuffer
	m_sfRingBuffer.cyclic_write(sfNewSamples, iNumSamples, 
		iCurrentReadCursor, m_iWriteCursor);

	// Schreibpointer weiter setzen
	m_iWriteCursor = ( m_iWriteCursor + iNumSamples ) % m_sfRingBuffer.GetLength();

	// Gibt freien Platz im RingBuffer zurueck
	if (iCurrentReadCursor > m_iWriteCursor) {
		return m_iWriteCursor - iCurrentReadCursor;
	}
	else {
		return m_sfRingBuffer.GetLength() - m_iWriteCursor + iCurrentReadCursor;
	}	
}

int CITANetAudioStream::GetRingbufferFreeSamples()
{
	int iFreeSamples = ( m_iWriteCursor - m_iReadCursor + GetRingBufferSize() ) % GetRingBufferSize();
	return iFreeSamples;
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
