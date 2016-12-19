#include <ITANetAudioClient.h>

#include <ITANetAudioMessage.h>
#include <ITANetAudioProtocol.h>
#include <ITANetAudioStream.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioClient::CITANetAudioClient( CITANetAudioStream* pParent )
	: m_pParent( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );
}

CITANetAudioClient::~CITANetAudioClient()
{
	if( m_pConnection )
	{
		m_pMessage->ResetMessage();
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteAnswer();
	}

	delete m_pMessage;
}

bool CITANetAudioClient::Connect( const std::string& sAddress, int iPort )
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
	
	m_pMessage->SetConnection( m_pConnection );

	m_pMessage->ResetMessage();
	m_pMessage->WriteClientOpen();
	m_pMessage->WriteMessage();
	m_pMessage->ReadAnswer();

	Run();

	return true;
}

void CITANetAudioClient::Disconnect()
{
	m_bStopIndicated = true;
	StopGently( true );

	delete m_pConnection;
	m_pConnection = NULL;

	m_bStopIndicated = false;
}

bool CITANetAudioClient::LoopBody()
{
	if( m_bStopIndicated )
		return true;

	// Receive message
	m_pMessage->ReadMessage();
	switch( m_pMessage->GetMessageType() )
	{
	case CITANetAudioProtocol::NP_INVALID:
		break;
	case CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES:
		/*
		int iNumSamples = m_pMessage->ReadSamples( m_sfReceivingBuffer );
		m_pParent->Transmit( m_sfReceivingBuffer, iNumSamples );
		int iFreeSamples = m_pParent->GetRingBufferFreeSamples();
		m_pMessage->WriteFreeRingBufferSamples( iFreeSamples );
		m_pMessage->WriteAnswer();
		*/
		break;
	}
}


bool CITANetAudioClient::GetIsConnected() const
{
	if( m_pConnection )
		return true;
	else
		return false;
}
