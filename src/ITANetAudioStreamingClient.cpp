#include "ITANetAudioStreamingClient.h"

#include "ITANetAudioClient.h"
#include "ITANetAudioMessage.h"
#include "ITANetAudioProtocol.h"

#include <ITAClock.h>
#include <ITADataLog.h>
#include <ITANetAudioStream.h>
#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/Concurrency/VistaPriority.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>

//! Audio streaming log item
struct ITAClientLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t"
		   << "WorldTimeStamp";
		os << "\t"
		   << "ProtocolStatus";
		os << "\t"
		   << "TransmittedRingBufferFreeSamples";
		os << "\t"
		   << "FreeSamples";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << sProtocolStatus;
		os << "\t" << ( bTransmittedRingBufferFreeSamples ? "true" : "false" );
		os << "\t" << iFreeSamples;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	std::string sProtocolStatus;
	bool bTransmittedRingBufferFreeSamples;
	int iFreeSamples;
};

class ITABufferedDataLoggerImplClient : public ITABufferedDataLogger<ITAClientLog>
{
};


CITANetAudioStreamingClient::CITANetAudioStreamingClient( CITANetAudioStream* pParent )
    : m_pStream( pParent )
    , m_pConnection( NULL )
    , m_bStopIndicated( false )
    , m_bStopped( false )
    , m_iStreamingBlockId( 0 )
    , m_dServerClockSyncRequestTimeInterval( 0.1f )
    , m_dServerClockSyncLastSyncTime( 0.0f )
    , m_bDebuggingEnabled( false )
{
	m_pClient = new CITANetAudioClient( );

	m_sfReceivingBuffer.init( pParent->GetNumberOfChannels( ), pParent->GetRingBufferSize( ), false );

	m_pMessage = new CITANetAudioMessage( VistaSerializingToolset::SWAPS_MULTIBYTE_VALUES );

	m_pClientLogger = new ITABufferedDataLoggerImplClient( );
	SetClientLoggerBaseName( "ITANetAudioStreamingClient" );

	// Careful with this.
	// SetPriority( VistaPriority::VISTA_MID_PRIORITY );
}

CITANetAudioStreamingClient::~CITANetAudioStreamingClient( )
{
	if( GetIsConnected( ) )
		Disconnect( );

	StopGently( false );

	if( GetIsDebuggingEnabled( ) )
	{
		vstr::out( ) << "[ ITANetAudioStreamingClient ] Processing statistics: " << m_swTryReadBlockStats.ToString( ) << std::endl;
		vstr::out( ) << "[ ITANetAudioStreamingClient ] Try-read access statistics: " << m_swTryReadAccessStats.ToString( ) << std::endl;
	}
	else
		m_pClientLogger->setOutputFile( "" ); // disable output

	delete m_pClientLogger;
	delete m_pClient;
	delete m_pMessage;
}

bool CITANetAudioStreamingClient::Connect( const std::string& sAddress, const int iPort, const bool bUseUDP )
{
	if( GetIsConnected( ) )
		return false;

	if( !m_pClient->Connect( sAddress, iPort, bUseUDP ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not connect to " + sAddress );

	if( !m_pClient->GetIsConnected( ) )
		return false;

	m_pConnection = m_pClient->GetConnection( );

	m_pMessage->ResetMessage( );
	m_pMessage->SetConnection( m_pConnection );

	CITANetAudioProtocol::StreamingParameters oParams;
	oParams.dSampleRate     = m_pStream->GetSampleRate( );
	oParams.iBlockSize      = m_pStream->GetBlocklength( );
	oParams.iChannels       = m_pStream->GetNumberOfChannels( );
	oParams.iRingBufferSize = m_pStream->GetRingBufferSize( );

	// Validate streaming parameters of server and client
	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_OPEN );
	m_pMessage->WriteStreamingParameters( oParams );
	m_pMessage->WriteMessage( );
	m_pMessage->ResetMessage( );

	while( !m_pMessage->ReadMessage( 0 ) )
		;

	int iMsgType = m_pMessage->GetMessageType( );
	if( iMsgType == CITANetAudioProtocol::NP_SERVER_OPEN )
	{
		// Clock sync vars
		m_dServerClockSyncRequestTimeInterval = m_pMessage->ReadDouble( );
		m_dServerClockSyncLastSyncTime        = 0;

		Run( );

		return true;
	}
	else if( iMsgType == CITANetAudioProtocol::NP_SERVER_REFUSED_INVALID_PARAMETERS )
	{
		ITA_EXCEPT1( INVALID_PARAMETER, "Server refused connection due to invalid streaming parameters (mismatching block size or sampling rate)" );
	}
	else
	{
		ITA_EXCEPT1( INVALID_PARAMETER, "Server connection could not be established, unrecognized answer received." );
	}

	return false;
}

bool CITANetAudioStreamingClient::LoopBody( )
{
	if( !GetIsConnected( ) )
		return true;

	ITAClientLog oLog;
	oLog.uiBlockId                         = ++m_iStreamingBlockId;
	oLog.iFreeSamples                      = m_pStream->GetRingBufferFreeSamples( );
	oLog.sProtocolStatus                   = CITANetAudioProtocol::GetNPMessageID( CITANetAudioProtocol::NP_CLIENT_IDLE );
	oLog.bTransmittedRingBufferFreeSamples = false;

	oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );

	if( m_bStopIndicated && !m_bStopped )
	{
		m_pMessage->ResetMessage( );
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_CLOSE );
		m_pMessage->WriteMessage( );

		while( true )
		{
			m_pMessage->ResetMessage( );
			m_pMessage->ReadMessage( 0 );
			int iMsgType = m_pMessage->GetMessageType( );
			if( iMsgType == CITANetAudioProtocol::NP_SERVER_CLOSE )
				break;
		}

		m_bStopped = true;
		m_pMessage->SetConnection( NULL );

		while( GetIsConnected( ) )
			VistaTimeUtils::Sleep( 100 );

		return true;
	}
	// Send message to server that samples can be received

	// Try-read message (blocking for a timeout of 1ms)
	m_pMessage->ResetMessage( );
	m_swTryReadBlockStats.start( );
	m_swTryReadAccessStats.start( );
	if( m_pMessage->ReadMessage( 1 ) )
	{
		m_swTryReadAccessStats.stop( );

		int iMsgType = m_pMessage->GetMessageType( );
		switch( iMsgType )
		{
			case CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES:
				m_pMessage->ReadSampleFrame( &m_sfReceivingBuffer );
				if( m_pStream->GetRingBufferFreeSamples( ) >= m_sfReceivingBuffer.GetLength( ) )
					m_pStream->Transmit( m_sfReceivingBuffer, m_sfReceivingBuffer.GetLength( ) );
#ifdef NET_AUDIO_SHOW_TRAFFIC
				vstr::out( ) << "[ITANetAudioStreamingClient] Received " << m_sfReceivingBuffer.GetLength( ) << " samples" << std::endl;
#endif
				break;

			case CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES:
				m_pMessage->ReadBool( );
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

		// Also log message type on incoming message
		oLog.sProtocolStatus = CITANetAudioProtocol::GetNPMessageID( iMsgType );
	}
	if( m_swTryReadBlockStats.started( ) )
		m_swTryReadBlockStats.stop( );

	// Send ring buffer free samples occasionally (as requested by server)
	const double dNow = ITAClock::getDefaultClock( )->getTime( );
	if( ( m_dServerClockSyncLastSyncTime + m_dServerClockSyncRequestTimeInterval ) < dNow )
	{
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES );
		m_pMessage->WriteInt( m_pStream->GetRingBufferFreeSamples( ) );
		m_pMessage->WriteMessage( );
		m_dServerClockSyncLastSyncTime         = dNow;
		oLog.bTransmittedRingBufferFreeSamples = true;
	}

	m_pClientLogger->log( oLog );

	return false;
}

std::string CITANetAudioStreamingClient::GetClientLoggerBaseName( ) const
{
	return m_sClientLoggerBaseName;
}

void CITANetAudioStreamingClient::SetClientLoggerBaseName( const std::string& sBaseName )
{
	if( GetIsConnected( ) )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming and logging already started." );

	m_sClientLoggerBaseName = sBaseName;

	m_pClientLogger->setOutputFile( m_sClientLoggerBaseName + "_Client.log" );
	m_pMessage->SetMessageLoggerBaseName( GetClientLoggerBaseName( ) + "_Messages" );
}

void CITANetAudioStreamingClient::SetDebuggingEnabled( const bool bEnabled )
{
	m_bDebuggingEnabled = bEnabled;
	m_pMessage->SetDebuggingEnabled( bEnabled );
}

bool CITANetAudioStreamingClient::GetIsDebuggingEnabled( ) const
{
	return m_bDebuggingEnabled;
}

bool CITANetAudioStreamingClient::GetIsConnected( ) const
{
	return m_pClient->GetIsConnected( );
}

void CITANetAudioStreamingClient::Disconnect( )
{
	m_bStopIndicated = true;
	StopGently( true );

	while( !m_bStopped )
		VistaTimeUtils::Sleep( 100 );

	m_pConnection = NULL;
	m_pMessage->ClearConnection( );
	m_pClient->Disconnect( );
	m_bStopIndicated = false;
	m_bStopped       = false;
}
