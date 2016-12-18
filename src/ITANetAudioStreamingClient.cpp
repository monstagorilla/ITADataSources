#include <ITANetAudioStreamingCLient.h>
#include <ITANetAudioStream.h>
#include <ITANetAudioProtocol.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_pParent( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	if( m_pConnection )
	{
		int iMessageType = CITANetAudioProtocol::NP_CLIENT_CLOSE;
		m_pConnection->Send( &iMessageType, sizeof( int ) );
	}
}

bool CITANetAudioStreamingClient::Connect( const std::string& sAddress, int iPort )
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

	int iMessageType = CITANetAudioProtocol::NP_CLIENT_OPEN;
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
}

void CITANetAudioStreamingClient::Disconnect()
{
	m_bStopIndicated = true;
	StopGently( true );

	delete m_pConnection;
	m_pConnection = NULL;

	m_bStopIndicated = false;
}

bool CITANetAudioStreamingClient::LoopBody()
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
}


bool CITANetAudioStreamingClient::GetIsConnected()
{
	if( m_pConnection )
		return true;
	else
		return false;
}
