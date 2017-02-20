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
	m_oParams.dSampleRate = pParent->GetSampleRate( );
	m_oParams.iBlockSize = pParent->GetBlocklength( );
	m_oParams.iRingBufferSize = pParent->GetRingBufferSize( );

	std::string paras = std::string("NetAudioLogClient") + std::string("_BS") + std::to_string(pParent->GetBlocklength()) + std::string("_Ch") + std::to_string(pParent->GetNumberOfChannels()) + std::string(".txt");
	m_pClientLogger = new ITABufferedDataLoggerImplClient( );
	m_pClientLogger->setOutputFile(paras);
	iStreamingBlockId = 0;
	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	//try{
		if (m_pConnection->GetIsConnected())
		{
			m_pMessage->ResetMessage();
			m_pMessage->SetConnection(m_pConnection);
			m_pMessage->SetMessageType(CITANetAudioProtocol::NP_CLIENT_CLOSE);
			m_pMessage->WriteBool(true);
			m_pMessage->WriteMessage();
			//m_pClient->Disconnect();
		}
		delete m_pClientLogger;
	//}
	//catch (ITAException e){
	//	std::cout << e << std::endl;
	//}
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
	m_pMessage->WriteMessage( );
	m_pMessage->ResetMessage( );

	while ( !m_pMessage->ReadMessage( 0 ) );
	
	assert( m_pMessage->GetMessageType( ) == CITANetAudioProtocol::NP_SERVER_OPEN );
	bool bOK = m_pMessage->ReadBool();
	
	if( !bOK )
		ITA_EXCEPT1( INVALID_PARAMETER, "Streaming server declined connection, detected streaming parameter mismatch." );

	Run();

	return true;
}

bool CITANetAudioStreamingClient::LoopBody()
{
	ITAClientLog oLog;
	oLog.uiBlockId = ++iStreamingBlockId;

	if( m_bStopIndicated )
		return true;

	// Send message to server that samples can be received

	m_pMessage->ResetMessage( );
	// Read answer 
	if ( m_pMessage->ReadMessage( 0 ) )
	{
		int iMsgType = m_pMessage->GetMessageType( );
		switch ( iMsgType )
		{
			case CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES:
				// Receive samples from net message and forward them to the stream ring buffer

				m_pMessage->ReadSampleFrame( &m_sfReceivingBuffer );
				if ( m_pStream->GetRingBufferFreeSamples( ) >= m_sfReceivingBuffer.GetLength( ) )
					m_pStream->Transmit( m_sfReceivingBuffer, m_sfReceivingBuffer.GetLength( ) );
				//else 
				// Fehler

				break;
			case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES:
				m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES );
				m_pMessage->WriteInt( m_pStream->GetRingBufferFreeSamples( ) );
				m_pMessage->WriteMessage( );
				break;
			case CITANetAudioProtocol::NP_SERVER_CLOSE:
				Disconnect( );
				break;
			default:
				vstr::out( ) << "[ITANetAudioStreamingServer] Unkown protocol type : " << iMsgType << std::endl;
				break;
		}
		oLog.iChannel = m_pStream->GetNumberOfChannels();
		oLog.iProtocolStatus = iMsgType;
		oLog.iFreeSamples = m_pStream->GetRingBufferFreeSamples();
		oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
		m_pClientLogger->log( oLog );
	}
	return false;
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

	//delete m_pConnection;
	m_pConnection = NULL;

	m_bStopIndicated = false;
}
