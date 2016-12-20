#include <ITANetAudioStreamingCLient.h>
#include <ITANetAudioStream.h>
#include <ITANetAudioProtocol.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_pParent( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
	m_pClient = new CITANetAudioClient( this );

	m_oClientParams.iChannels = pParent->GetNumberOfChannels();
	m_oClientParams.dSampleRate = pParent->GetSampleRate();
	m_oClientParams.iBlockSize = pParent->GetBlocklength();
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
	if( GetIsConnected )
		return false;
	
	if( !m_pClient->Connect( sAddress, iPort ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to " + sAddress );
	
	m_pConnection = m_pClient->GetConnection();

	m_pMessage->SetConnection( m_pConnection );

	// Validate streaming parameters of server and client
	m_pMessage->ResetMessage();
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pMessage->WriteStreamingParameters( m_oClientParams );
	m_pMessage->WriteMessage();

	m_pMessage->ReadAnswer();
	CITANetAudioProtocol::StreamingParameters oServerParams = m_pMessage->ReadStreamingParameters();
	if( oServerParams == m_oClientParams )
		m_oServerParams = oServerParams;
	else
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
	}
}

bool CITANetAudioStreamingClient::GetIsConnected() const
{
	return m_pClient->GetIsConnected();
}
