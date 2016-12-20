#include <ITANetAudioStreamingClient.h>

#include <ITANetAudioClient.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioStream.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_pStream( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
	m_pClient = new CITANetAudioClient();

	m_oParams.iChannels = pParent->GetNumberOfChannels();
	m_oParams.dSampleRate = pParent->GetSampleRate();
	m_oParams.iBlockSize = pParent->GetBlocklength();
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	if( m_pConnection )
	{
		m_pMessage->ResetMessage();
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteAnswer();
	}
}

bool CITANetAudioStreamingClient::Connect( const std::string& sAddress, int iPort )
{
	if( GetIsConnected() )
		return false;
	
	if( !m_pClient->Connect( sAddress, iPort ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to " + sAddress );
	
	m_pConnection = m_pClient->GetConnection();

	m_pMessage->SetConnection( m_pConnection );

	// Validate streaming parameters of server and client
	m_pMessage->ResetMessage();
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pMessage->WriteStreamingParameters( m_oParams );
	m_pMessage->WriteMessage();

	m_pMessage->ReadAnswer();
	// NOTE: Wieso speichern wir beie paras, nach einmal vergleichen, sollten doch sowieso beide gleich sein
	CITANetAudioProtocol::StreamingParameters oServerParams = m_pMessage->ReadStreamingParameters();
	if (!(oServerParams == m_oParams))
		ITA_EXCEPT1( INVALID_PARAMETER, "Streaming parameters of network audio server and client do not match." );

	Run();

	return true;
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
	case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE :
		break;
	}
}

bool CITANetAudioStreamingClient::GetIsConnected() const
{
	return m_pClient->GetIsConnected();
}
