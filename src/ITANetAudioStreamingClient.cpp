#include <ITANetAudioStreamingClient.h>

#include <ITANetAudioClient.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioStream.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_oBlockIncrementEvent( VistaThreadEvent::WAITABLE_EVENT )
	, m_pStream( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
	m_pStreamProbe = new ITAStreamProbe( pParent, "output.wav" );

	m_pClient = new CITANetAudioClient();

	m_oParams.iChannels = pParent->GetNumberOfChannels();
	m_oParams.dSampleRate = pParent->GetSampleRate();
	m_oParams.iBlockSize = pParent->GetBlocklength();

	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	if( m_pConnection )
	{
		m_pMessage->ResetMessage();
		m_pMessage->SetConnection( m_pConnection );
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteMessage();
		m_pMessage->ReadAnswer();
		m_pClient->Disconnect();
	}
}

bool CITANetAudioStreamingClient::Connect( const std::string& sAddress, int iPort )
{
	if( GetIsConnected() )
		return false;
	
	if( !m_pClient->Connect( sAddress, iPort ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to " + sAddress );
	
	m_pConnection = m_pClient->GetConnection();

	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );

	// Validate streaming parameters of server and client
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pMessage->WriteStreamingParameters( m_oParams );
	m_pMessage->WriteMessage();

	m_pMessage->ReadAnswer();
	assert( m_pMessage->GetAnswerType() == CITANetAudioProtocol::NP_SERVER_OPEN );
	bool bOK = m_pMessage->ReadBool();
	
	/* Not yet
	CITANetAudioProtocol::StreamingParameters oServerParams = m_pMessage->ReadStreamingParameters();
	if (!(oServerParams == m_oParams))
		ITA_EXCEPT1( INVALID_PARAMETER, "Streaming parameters of network audio server and client do not match." );
	*/
	if( !bOK )
		ITA_EXCEPT1( INVALID_PARAMETER, "Streaming server declined connection, detected streaming parameter mismatch." );

	Run();

	return true;
}

bool CITANetAudioStreamingClient::LoopBody()
{
	if( m_bStopIndicated )
		return true;

	// Send message to server that samples can be received
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES );
	int iFreeSamplesUntilAllowedReached = m_pStream->GetAllowedLatencySamples() - m_pStream->GetRingBufferAvailableSamples();
	if( iFreeSamplesUntilAllowedReached < 0 )
		iFreeSamplesUntilAllowedReached = 0;
	m_pMessage->WriteInt( iFreeSamplesUntilAllowedReached );
	m_pMessage->WriteMessage();

	// Wait for answer of server
	m_pMessage->ReadAnswer();
	int iAnswerType = m_pMessage->GetAnswerType();
	switch( iAnswerType )
	{

	case CITANetAudioProtocol::NP_INVALID:
		// Something went wrong
		std::cerr << "Received invalid message type" << std::endl;
		break;

	case CITANetAudioProtocol::NP_SERVER_WAITING_FOR_TRIGGER:
		// Wait until block increment is triggered by audio context (more free samples in ring buffer)
		//std::cout << "Will wait for block increment" << std::endl;
		m_oBlockIncrementEvent.WaitForEvent( true );
		break;

	case CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES:
		// Receive samples from net message and forward them to the stream ring buffer

		m_pMessage->ReadSampleFrame( &m_sfReceivingBuffer );
		//std::cout << "Receiving " << m_sfReceivingBuffer.GetLength() << " samples from streaming server" << std::endl;
		if ( m_pStream->GetRingBufferFreeSamples( ) >= m_sfReceivingBuffer.GetLength( ) )
			m_pStream->Transmit( m_sfReceivingBuffer, m_sfReceivingBuffer.GetLength( ) );
		//else 
			// Fehler
		
		break;
	case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE :
		break;
	}

	return true;
}

void CITANetAudioStreamingClient::TriggerBlockIncrement()
{
	m_oBlockIncrementEvent.SignalEvent();
}

bool CITANetAudioStreamingClient::GetIsConnected() const
{
	return m_pClient->GetIsConnected();
}

void CITANetAudioStreamingClient::Disconnect()
{
	m_bStopIndicated = true;
	StopGently( true );

	delete m_pConnection;
	m_pConnection = NULL;

	m_bStopIndicated = false;
}
