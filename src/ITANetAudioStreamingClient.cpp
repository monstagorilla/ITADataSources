#include <ITANetAudioStreamingClient.h>

#include <ITANetAudioClient.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioStream.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaTimeUtils.h>

CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_pStream( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
	, m_bStopped( false )
{
	m_pClient = new CITANetAudioClient();

	m_oParams.iChannels = pParent->GetNumberOfChannels();
	m_oParams.dSampleRate = pParent->GetSampleRate();
	m_oParams.iBlockSize = pParent->GetBlocklength();

	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	if( GetIsConnected() )
		Disconnect();

	StopGently( true );

	delete m_pClientLogger;
	delete m_pClient;
	delete m_pMessage;
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
	
	if( !bOK )
		ITA_EXCEPT1( INVALID_PARAMETER, "Streaming server declined connection, detected streaming parameter mismatch." );

	Run();

	return true;
}

bool CITANetAudioStreamingClient::LoopBody()
{
	if( !GetIsConnected() )
		return true;

	ITAClientLog oLog;
	oLog.uiBlockId = ++iStreamingBlockId;

	if( m_bStopIndicated && !m_bStopped )
	{
		m_pMessage->ResetMessage();
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteMessage();
		m_bStopped = true;
		
		m_pMessage->SetConnection( NULL );

		while( GetIsConnected() )
			VistaTimeUtils::Sleep( 100 );

		return true;
	}
	
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES );
	int iFreeSamplesUntilAllowedReached = m_pStream->GetAllowedLatencySamples() - m_pStream->GetRingBufferAvailableSamples();
	if( iFreeSamplesUntilAllowedReached < 0 )
		iFreeSamplesUntilAllowedReached = 0;
	m_pMessage->WriteInt( iFreeSamplesUntilAllowedReached );
	m_pMessage->WriteMessage();

	// Read answer (blocking)
	m_pMessage->ResetMessage( );
	if ( m_pMessage->ReadMessage( 0 ) )
	{
		int iMsgType = m_pMessage->GetMessageType( );

	case CITANetAudioProtocol::NP_INVALID:
		// Something went wrong
		vstr::err() << "Received invalid message type" << std::endl;
		break;

	case CITANetAudioProtocol::NP_SERVER_WAITING_FOR_TRIGGER:
		// Wait until block increment is triggered by audio context (more free samples in ring buffer)
		m_oBlockIncrementEvent.WaitForEvent( true );
		break;

	case CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES:
		// Receive samples from net message and forward them to the stream ring buffer

		m_pMessage->ReadSampleFrame( &m_sfReceivingBuffer );
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

bool CITANetAudioStreamingClient::GetIsConnected() const
{
	return m_pClient->GetIsConnected();
}

void CITANetAudioStreamingClient::Disconnect()
{
	m_bStopIndicated = true;

	while( !m_bStopped )
		VistaTimeUtils::Sleep( 100 );
	
	m_pConnection = NULL;
	m_pClient->Disconnect();
	m_bStopIndicated = false;
	m_bStopped = false;
}
