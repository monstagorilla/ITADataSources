#include <ITANetAudioStream.h>

// ITA includes
#include <ITAException.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioProtocol.h>

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
		if( m_pConnection )
		{
			int iMessageType = NET_MESSAGE_CLOSE;
			m_pConnection->Send( &iMessageType, sizeof( int ) );
		}
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


	inline bool GetIsConnected()
	{
		if( m_pConnection )
			return true;
		else
			return false;
	};

private:
	CITANetAudioStream* m_pParent;

	VistaConnectionIP* m_pConnection;
	CITANetAudioProtocol* m_pProtocol;
	CITANetAudioMessage* m_pMessage;

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

CITANetAudioStream::~CITANetAudioStream()
{
	delete m_pNetAudioProducer;
}

bool CITANetAudioStream::Connect( const std::string& sAddress, int iPort )
{
	return m_pNetAudioProducer->Connect( sAddress, iPort );
}

bool CITANetAudioStream::GetIsConnected() const
{
	return m_pNetAudioProducer->GetIsConnected();
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
