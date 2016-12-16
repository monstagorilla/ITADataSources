#include <ITANetAudioStream.h>

// ITA includes
#include <ITAException.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
//#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

// STL
#include <cmath>

class CITANetAudioStreamConnection : public VistaThreadLoop
{
public:
	enum MessageType
	{
		NET_MESSAGE_NONE = 0,
		NET_MESSAGE_OPEN,
		NET_MESSAGE_CLOSE,
		NET_MESSAGE_SAMPLES,
	};

	inline CITANetAudioStreamConnection( CITANetAudioStream* pParent )
		: m_pParent( pParent )
		, m_pConnection( NULL )
		, m_bStopIndicated( false )
	{
	};

	inline bool Connect( const std::string& sAddress, int iPort )
	{
		if( m_pConnection )
			ITA_EXCEPT1( MODAL_EXCEPTION, "This net stream is already connected" );

		// Attempt to connect and check parameters
		m_pConnection = new VistaConnectionIP( VistaConnectionIP::CT_TCP, sAddress, iPort );
		if( !m_pConnection->GetIsConnected() )
		{
			delete m_pConnection;
			m_pConnection = NULL;
			return false;
		}

		int iMessageType = NET_MESSAGE_OPEN;
		m_pConnection->Send( &iMessageType, sizeof( int ) );

		//Vistabytebuffer
		int iNumChannels = ( int ) m_pParent->GetNumberOfChannels();
		m_pConnection->Send( &iNumChannels, sizeof( int ) );
		double dSampleRate = m_pParent->GetSampleRate();
		m_pConnection->Send( &dSampleRate, sizeof( double ) );
		int iBlockLength = ( int ) m_pParent->GetBlocklength();
		m_pConnection->Send( &iBlockLength, sizeof( int ) );
		int iRingBufferSize = ( int ) m_pParent->GetRingBufferSize();
		m_pConnection->Send( &iRingBufferSize, sizeof( int ) );
		m_pConnection->WaitForSendFinish();

		int iServerMessageType;
		m_pConnection->Receive( &iServerMessageType, sizeof( int ) );

		Run();
	};

	inline void Disconnect()
	{
		m_bStopIndicated = true;
		StopGently( true );

		delete m_pConnection;
		m_pConnection = NULL;

		m_bStopIndicated = false;
	};

	inline ~CITANetAudioStreamConnection()
	{
		int iMessageType = NET_MESSAGE_CLOSE;
		m_pConnection->Send( &iMessageType, sizeof( int ) );
	};

	inline bool LoopBody()
	{
		if( m_bStopIndicated )
			return true;

		// Receive messages
		while( true )
		{
			m_pConnection->Receive( NULL, 0 ); // @todo: receive messages and react

			int iNumSamples = 12;
			if( true )
				m_pParent->Transmit( m_sfReceivingBuffer, iNumSamples );
		}
	};

private:
	VistaConnectionIP* m_pConnection;
	CITANetAudioStream* m_pParent;
	ITASampleFrame m_sfReceivingBuffer;	
	bool m_bStopIndicated;
};

CITANetAudioStream::CITANetAudioStream( int iChannels, double dSamplingRate, int iBufferSize, int iRingBufferCapacity )
	: m_sfOutputStreamBuffer( iChannels, iBufferSize, true )
	, m_dSampleRate( dSamplingRate )
	, m_sfRingBuffer( iChannels, iRingBufferCapacity, true )
	
{
	m_pNetAudioProducer = new CITANetAudioStreamConnection( this );
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	return m_pNetAudioProducer->Connect( sAddress, iPort );
}

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetAudioProducer;
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
		
	// Threadsave programmieren

	
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
