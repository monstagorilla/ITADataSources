#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITANetAudioMessage.h>

// ITA includes
#include <ITADataSource.h>
#include <ITANetAudioMessage.h>
#include <ITAException.h>
#include <ITAStreamInfo.h>
#include <ITAClock.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>
#include <VistaBase/VistaStreamUtils.h>
#include <ITADataLog.h>

// STL
#include <cmath>
#include <cassert>

struct ITAServerLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "ProtocolStatus";
		os << "\t" << "FreeSamples";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << iProtocolStatus;
		os << "\t" << iFreeSamples;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	int iProtocolStatus; //!< ... usw
	int iFreeSamples;
};

class ITABufferedDataLoggerImplServer : public ITABufferedDataLogger < ITAServerLog > {};

CITANetAudioStreamingServer::CITANetAudioStreamingServer( )
: m_pInputStream( NULL )
, m_iUpdateStrategy( AUTO )
, m_pConnection( NULL )
, m_pNetAudioServer( new CITANetAudioServer( ) )
{
	iServerBlockId = 0;
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	if ( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	if ( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection( );

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag( ) );
	m_pMessage->ResetMessage( );
	m_pMessage->SetConnection( m_pConnection );
	while ( !m_pMessage->ReadMessage( 0 ) ); //blocking

	assert( m_pMessage->GetMessageType( ) == CITANetAudioProtocol::NP_CLIENT_OPEN );
	CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters( );

	bool bOK = false;
	m_oServerParams.iRingBufferSize = oClientParams.iRingBufferSize;
	m_iClientRingBufferFreeSamples = m_oServerParams.iRingBufferSize;
	if ( m_oServerParams == oClientParams )
	{
		bOK = true;
#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters matched. Will resume with streaming" << std::endl;
	}
	else
	{
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters mismatch detected. Will notify client and stop." << std::endl;
#endif
	}

	std::string paras = std::string( "NetAudioLogServer" ) + std::string( "_BS" ) + std::to_string( m_oServerParams.iBlockSize ) + std::string( "_Ch" ) + std::to_string( m_oServerParams.iChannels ) + std::string( ".txt" );
	m_pServerLogger = new ITABufferedDataLoggerImplServer( );
	m_pServerLogger->setOutputFile( paras );

	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_OPEN );
	m_pMessage->WriteBool( bOK );
	m_pMessage->WriteMessage( );

	if ( bOK )
		Run( );

	return bOK;
}

bool CITANetAudioStreamingServer::LoopBody( )
{
	m_pMessage->ResetMessage( );
	ITAServerLog oLog;
	oLog.uiBlockId = ++iServerBlockId;
	int iMsgType;
	// Sending Samples 
	if ( m_iClientRingBufferFreeSamples >= int( m_pInputStream->GetBlocklength( ) ) )
	{
		// Send Samples
		while ( m_iClientRingBufferFreeSamples >= int( m_pInputStream->GetBlocklength( ) ) )
		{
			for ( int i = 0; i < int( m_pInputStream->GetNumberOfChannels( ) ); i++ )
			{
				ITAStreamInfo oStreamInfo;
				oStreamInfo.nSamples = m_sfTempTransmitBuffer.GetLength( );
				const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
				if ( pfData != 0 )
					m_sfTempTransmitBuffer[ i ].write( pfData, m_sfTempTransmitBuffer.GetLength( ) );
			}
			m_pInputStream->IncrementBlockPointer( );
			iMsgType = CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES;
			m_pMessage->SetMessageType( iMsgType );
			m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
			m_pMessage->WriteMessage( );
			m_iClientRingBufferFreeSamples -= m_sfTempTransmitBuffer.GetLength( );
#ifdef NET_AUDIO_SHOW_TRAFFIC
			vstr::out( ) << "[ITANetAudioStreamingServer] Transmitted " << m_sfTempTransmitBuffer.GetLength( ) << " samples for "
				<< m_pInputStream->GetNumberOfChannels( ) << " channels" << std::endl;
#endif
		}
	}
	else
	{
		// Waiting for Trigger
		iMsgType = CITANetAudioProtocol::NP_SERVER_GET_RINGBUFFER_FREE_SAMPLES;
		m_pMessage->SetMessageType( iMsgType );
		m_pMessage->WriteMessage( );

#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out( ) << "[ITANetAudioStreamingServer] Not enough free samples in client buffer, requesting a trigger when more free samples available" << std::endl;
#endif
	}

	oLog.iFreeSamples = m_iClientRingBufferFreeSamples;
	oLog.iProtocolStatus = iMsgType;
	oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
	m_pServerLogger->log( oLog );

	// Try to Empfange Daten
	//m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ResetMessage( );

	if ( m_pMessage->ReadMessage( 1 ) )
	{
		ITAServerLog oLog;
		oLog.uiBlockId = ++iServerBlockId;
		int iMsgType = m_pMessage->GetMessageType( );
		switch ( iMsgType )
		{
			case CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES:
			{
				m_iClientRingBufferFreeSamples = m_pMessage->ReadInt( );	
				break;
			}
			case CITANetAudioProtocol::NP_CLIENT_CLOSE:
			{
				//m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_CLOSE );
				//m_pMessage->WriteAnswer();
				StopGently( false );
				m_pConnection = NULL;
				Stop( );
				break;
			}
			default:
			{
				vstr::out( ) << "[ITANetAudioStreamingServer] Unkown protocol type : " << iMsgType << std::endl;
				break;
			}
		}
		oLog.iFreeSamples = m_iClientRingBufferFreeSamples;
		oLog.iProtocolStatus = iMsgType;
		oLog.dWorldTimeStamp = ITAClock::getDefaultClock( )->getTime( );
		m_pServerLogger->log( oLog );
	}


	return false;
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if ( VistaThreadLoop::IsRunning( ) )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels( ), m_pInputStream->GetBlocklength( ), true );
	m_oServerParams.dSampleRate = m_pInputStream->GetSampleRate( );
	m_oServerParams.iBlockSize = m_pInputStream->GetBlocklength( );
	m_oServerParams.iChannels = m_pInputStream->GetNumberOfChannels( );
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream( ) const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::GetNetStreamBlocklength( ) const
{
	return m_sfTempTransmitBuffer.GetLength( );
}

int CITANetAudioStreamingServer::GetNetStreamNumberOfChannels( ) const
{
	return m_sfTempTransmitBuffer.channels( );
}

void CITANetAudioStreamingServer::SetAutomaticUpdateRate( )
{
	m_iUpdateStrategy = AUTO;
}

bool CITANetAudioStreamingServer::IsClientConnected( ) const
{
	return m_pNetAudioServer->IsConnected( );
}

std::string CITANetAudioStreamingServer::GetNetworkAddress( ) const
{
	return m_pNetAudioServer->GetServerAddress( );
}

int CITANetAudioStreamingServer::GetNetworkPort( ) const
{
	return m_pNetAudioServer->GetNetworkPort( );
}

void CITANetAudioStreamingServer::Stop( )
{
	delete m_pServerLogger;
	m_pNetAudioServer->Stop( );
}
