#include <ITANetAudioStreamingClient.h>

#include <ITANetAudioClient.h>
#include <ITANetAudioMessage.h>
#include <ITANetAudioStream.h>
#include <ITADataLog.h>
#include <ITAClock.h>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaTimeUtils.h>

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
	: m_pStream( pParent )
	, m_pConnection( NULL )
	, m_bStopIndicated( false )
	, m_bStopped( false )
	, m_iStreamingBlockId( 0 )
	, m_dServerClockSyncRequestTimeInterval( 0.1f )
	, m_dServerClockSyncLastSyncTime( 0.0f )
{
	m_pClient = new CITANetAudioClient();
	m_oParams.iChannels = pParent->GetNumberOfChannels();
	m_oParams.dSampleRate = pParent->GetSampleRate();
	m_oParams.iBlockSize = pParent->GetBlocklength();
	m_oParams.iRingBufferSize = pParent->GetRingBufferSize();
		 
	m_sfReceivingBuffer.init( m_oParams.iChannels, m_oParams.iRingBufferSize, false );

	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );

	m_pClientLogger = new ITABufferedDataLoggerImplClient();
	SetClientLoggerBaseName( "ITANetAudioStreamingClient" );
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient()
{
	if( GetIsConnected() )
		Disconnect();

	StopGently( false );

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

	if( !m_pClient->GetIsConnected() )
		return false;

	m_pConnection = m_pClient->GetConnection();

	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );

	// Validate streaming parameters of server and client
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pMessage->WriteStreamingParameters( m_oParams );
	m_pMessage->WriteMessage();
	m_pMessage->ResetMessage();

	while( !m_pMessage->ReadMessage( 0 ) );

	assert( m_pMessage->GetMessageType() == CITANetAudioProtocol::NP_SERVER_OPEN );

	// Clock sync vars
	m_dServerClockSyncRequestTimeInterval = m_pMessage->ReadDouble();
	m_dServerClockSyncLastSyncTime = 0;
	
	Run();

	return true;
}

bool CITANetAudioStreamingClient::LoopBody()
{
	if( !GetIsConnected() )
		return true;

	ITAClientLog oLog;
	oLog.uiBlockId = ++m_iStreamingBlockId;
	oLog.iChannel = m_pStream->GetNumberOfChannels();
	oLog.iFreeSamples = m_pStream->GetRingBufferFreeSamples();
	oLog.iProtocolStatus = CITANetAudioProtocol::NP_CLIENT_IDLE;

	oLog.dWorldTimeStamp = ITAClock::getDefaultClock()->getTime();

	if( m_bStopIndicated && !m_bStopped )
	{
		m_pMessage->ResetMessage();
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteMessage();

		while( true )
		{
			m_pMessage->ResetMessage();
			m_pMessage->ReadMessage( 0 );
			int iMsgType = m_pMessage->GetMessageType();
			if( iMsgType == CITANetAudioProtocol::NP_SERVER_CLOSE )
				break;
		}

		m_bStopped = true;
		m_pMessage->SetConnection( NULL );

		while( GetIsConnected() )
			VistaTimeUtils::Sleep( 100 );

		return true;
	}

	// Try-read message (blocking for a timeout of 1ms)
	m_pMessage->ResetMessage();
	m_swTryReadStats.start();
	if( m_pMessage->ReadMessage( 1 ) )
	{
		m_swTryReadStats.stop();
		int iMsgType = m_pMessage->GetMessageType();
		switch( iMsgType )
		{
		case CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES:
			m_pMessage->ReadSampleFrame( &m_sfReceivingBuffer );
			if( m_pStream->GetRingBufferFreeSamples() >= m_sfReceivingBuffer.GetLength() )
				m_pStream->Transmit( m_sfReceivingBuffer, m_sfReceivingBuffer.GetLength() );
#ifdef NET_AUDIO_SHOW_TRAFFIC
			vstr::out() << "[ITANetAudioStreamingClient] Recived " << m_sfReceivingBuffer.GetLength() << " samples" << std::endl;
#endif
			break;

		case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES:
			m_pMessage->ReadBool();
			m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES );
			m_pMessage->WriteInt( m_pStream->GetRingBufferFreeSamples() );
			m_pMessage->WriteMessage();
			break;

		case CITANetAudioProtocol::NP_SERVER_CLOSE:
			Disconnect();
			break;

		default:
			vstr::out() << "[ITANetAudioStreamingServer] Unkown protocol type : " << iMsgType << std::endl;
			break;

		}

		// Also log message type on incoming message
		oLog.iProtocolStatus = iMsgType;

	}
	m_swTryReadStats.stop();

	m_pClientLogger->log( oLog );

	// Send ring buffer free samples occasionally (as requested by server)
	const double dNow = ITAClock::getDefaultClock()->getTime();
	if( ( m_dServerClockSyncLastSyncTime + m_dServerClockSyncRequestTimeInterval ) < dNow )
	{
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES );
		m_pMessage->WriteInt( m_pStream->GetRingBufferFreeSamples() );
		m_pMessage->WriteMessage();
		m_dServerClockSyncLastSyncTime = dNow;
	}
	
	return false;
}

std::string CITANetAudioStreamingClient::GetClientLoggerBaseName() const
{
	return m_sClientLoggerBaseName;
}

void CITANetAudioStreamingClient::SetClientLoggerBaseName( const std::string& sBaseName )
{
	m_sClientLoggerBaseName = sBaseName;

	m_pClientLogger->setOutputFile( m_sClientLoggerBaseName );
	m_pMessage->SetMessageLoggerBaseName( GetClientLoggerBaseName() + "_Messages" );
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
	m_pMessage->ClearConnection();
	m_pClient->Disconnect();
	m_bStopIndicated = false;
	m_bStopped = false;
}
