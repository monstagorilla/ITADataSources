#include <ITANetAudioStreamingClient.h>

#include <ITANetAudioClient.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioStream.h>
#include <ITADataLog.h>
#include <ITAClock.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaStreamUtils.h>

//! Audio streaming log item
struct ITAClientLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "ProtocolStatus";
		os << "\t" << "FreeSamples";
		os << "\t" << "Channel";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << iProtocolStatus;
		os << "\t" << iFreeSamples;
		os << "\t" << iChannel;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	int iProtocolStatus; //!< ... usw
	int iFreeSamples;
	int iChannel;

};

class ITABufferedDataLoggerImplClient : public ITABufferedDataLogger < ITAClientLog > {};


CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
	: m_oBlockIncrementEvent( VistaThreadEvent::WAITABLE_EVENT )
	, m_pStream( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
{
	m_pClient = new CITANetAudioClient();

	m_oParams.iChannels = pParent->GetNumberOfChannels();
	m_oParams.dSampleRate = pParent->GetSampleRate();
	m_oParams.iBlockSize = pParent->GetBlocklength();

	std::string paras = std::string("NetAudioLogClient") + std::string("_BS") + std::to_string(pParent->GetBlocklength()) + std::string("_Ch") + std::to_string(pParent->GetNumberOfChannels()) + std::string(".txt");
	m_pClientLogger = new ITABufferedDataLoggerImplClient( );
	m_pClientLogger->setOutputFile(paras);
	iStreamingBlockId = 0;
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	delete m_pClientLogger;
}

bool CITANetAudioStreamingClient::Connect( const std::string& sAddress, int iPort )
{
	if( GetIsConnected() )
		return false;
	
	if( !m_pClient->Connect( sAddress, iPort ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to " + sAddress );
	
	m_pConnection = m_pClient->GetConnection();

	m_pIncomingMessage = new CITANetAudioMessage( m_pConnection );
	m_pOutgoingMessage = new CITANetAudioMessage( m_pConnection );

	// Validate streaming parameters of server and client
	m_pOutgoingMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pOutgoingMessage->WriteStreamingParameters( m_oParams );
	m_pOutgoingMessage->WriteMessage();

	Run();

	return true;
}

bool CITANetAudioStreamingClient::LoopBody()
{
	ITAClientLog oLog;
	oLog.uiBlockId = ++iStreamingBlockId;

	if( m_bStopIndicated )
		return true;

	// Send message to server that (and how many) samples can be received
	m_pIncomingMessage->ResetMessage();
	if( !m_pIncomingMessage->TryReadMessage() )
	{
		int iFreeSamplesUntilAllowedReached = m_pStream->GetAllowedLatencySamples() - m_pStream->GetRingBufferAvailableSamples();
		oLog.iFreeSamples = iFreeSamplesUntilAllowedReached;
		if( iFreeSamplesUntilAllowedReached < 0 )
			iFreeSamplesUntilAllowedReached = 0;

		m_pOutgoingMessage->ResetMessage();
		m_pIncomingMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES );
		m_pOutgoingMessage->WriteInt( iFreeSamplesUntilAllowedReached );
		m_pOutgoingMessage->WriteMessage();

		return false;
	}

	int iIncomingMessageType = m_pIncomingMessage->GetMessageType();
	switch( iIncomingMessageType )
	{

	case CITANetAudioProtocol::NP_INVALID:
		// Something went wrong
		vstr::err() << "Received invalid message type" << std::endl;
		break;

	case CITANetAudioProtocol::NP_SERVER_CLOSE:
		Disconnect();
		break;

	case CITANetAudioProtocol::NP_SERVER_SEND_SAMPLES:
		// Receive samples from net message and forward them to the stream ring buffer

		m_pIncomingMessage->ReadSampleFrame( &m_sfReceivingBuffer );
		if ( m_pStream->GetRingBufferFreeSamples( ) >= m_sfReceivingBuffer.GetLength( ) )
			m_pStream->Transmit( m_sfReceivingBuffer, m_sfReceivingBuffer.GetLength( ) );
		//else 
			// Fehler
		
		break;
	case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE :
		break;
	}
	oLog.iChannel = m_pStream->GetNumberOfChannels();
	oLog.iProtocolStatus = iIncomingMessageType;
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
	m_pClientLogger->log( oLog );
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

	delete m_pIncomingMessage;
	delete m_pOutgoingMessage;
	
	m_pConnection = NULL;

	m_bStopIndicated = false;
}
