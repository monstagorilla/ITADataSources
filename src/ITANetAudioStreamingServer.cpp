#include <ITANetAudioStreamingServer.h>

#include "ITANetAudioServer.h"
#include "ITANetAudioMessage.h"

// ITA includes
#include <ITADataSource.h>
#include <ITAException.h>
#include <ITAStreamInfo.h>
#include <ITAClock.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>
#include <VistaInterProcComm/Concurrency/VistaPriority.h>
#include <VistaBase/VistaStreamUtils.h>
#include <ITADataLog.h>

// STL
#include <cmath>
#include <cassert>

struct CITAServerLog : public ITALogDataBase
{
	inline static std::ostream& outputDesc( std::ostream& os )
	{
		os << "BlockId";
		os << "\t" << "WorldTimeStamp";
		os << "\t" << "ProtocolStatus";
		os << "\t" << "EstimatedFreeSamples";
		os << "\t" << "TransmittedSamples";
		os << "\t" << "EstimatedCorrFactor";
		os << std::endl;
		return os;
	};

	inline std::ostream& outputData( std::ostream& os ) const
	{
		os << uiBlockId;
		os << "\t" << std::setprecision( 12 ) << dWorldTimeStamp;
		os << "\t" << sProtocolStatus;
		os << "\t" << iEstimatedFreeSamples;
		os << "\t" << iTransmittedSamples;
		os << "\t" << dEstimatedCorrFactor;
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	std::string sProtocolStatus;
	int iEstimatedFreeSamples;
	int iTransmittedSamples;
	double dEstimatedCorrFactor;
};

class CITABufferedDataLoggerImplServer : public ITABufferedDataLogger < CITAServerLog > {};

CITANetAudioStreamingServer::CITANetAudioStreamingServer()
	: m_pInputStream( NULL )
	, m_pConnection( NULL )
	, m_pNetAudioServer( new CITANetAudioServer() )
	, m_dLastTimeStamp( 0 )
	, m_iTargetLatencySamples( -1 )
	, m_sServerLogBaseName( "ITANetAudioStreamingServer" )
	, m_bDebuggingEnabled( false )
	, m_iMaxSendBlocks( 40 )
	, m_iServerBlockId( 0 )
	, m_iEstimatedClientRingBufferFreeSamples( 0 )
	, m_iClientRingBufferSize( 0 )
	, m_dEstimatedCorrFactor( 1 )
	, m_dStreamTimeStart( 0.0f )
	, m_nStreamSampleCounts( 0 )
{
	// Careful with this:
	//SetPriority( VistaPriority::VISTA_MID_PRIORITY );
}

CITANetAudioStreamingServer::~CITANetAudioStreamingServer()
{
	delete m_pNetAudioServer;


	if( GetIsDebuggingEnabled() )
	{
		vstr::out() << "[ ITANetAudioStreamingServer ] Processing statistics: " << m_swTryReadBlockStats.ToString() << std::endl;
		vstr::out() << "[ ITANetAudioStreamingServer ] Try-read access statistics: " << m_swTryReadAccessStats.ToString() << std::endl;
	}
	else
		m_pServerLogger->setOutputFile( "" ); // disables export

	delete m_pServerLogger;

}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, const int iPort, const double dTimeIntervalCientSendStatus, const bool bUseUDP /* = false */ )
{
	if( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	if( !m_pNetAudioServer->Start( sAddress, iPort, bUseUDP ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection();

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag() );
	m_pMessage->SetMessageLoggerBaseName( GetServerLogBaseName() + "_Messages" );

	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	while( !m_pMessage->ReadMessage( 0 ) ); //blocking

	assert( m_pMessage->GetMessageType() == CITANetAudioProtocol::NP_CLIENT_OPEN );
	CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters();

	CITANetAudioProtocol::StreamingParameters oServerParams;
	oServerParams.iRingBufferSize = oClientParams.iRingBufferSize;
	oServerParams.iBlockSize = m_pInputStream->GetBlocklength();
	oServerParams.dSampleRate = m_pInputStream->GetSampleRate();
	oServerParams.iChannels = m_pInputStream->GetNumberOfChannels();

	m_iEstimatedClientRingBufferFreeSamples = oServerParams.iRingBufferSize;
	m_iClientRingBufferSize = oClientParams.iRingBufferSize;
	m_iSendingBlockLength = oServerParams.iBlockSize;

	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), oServerParams.iRingBufferSize, true );

	m_pServerLogger = new CITABufferedDataLoggerImplServer();
	m_pServerLogger->setOutputFile( m_sServerLogBaseName + "_Server.log" );
	m_dLastTimeStamp = ITAClock::getDefaultClock()->getTime();

	if( oServerParams == oClientParams )
	{
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_OPEN );
		m_pMessage->WriteDouble( dTimeIntervalCientSendStatus );
		m_pMessage->WriteMessage();

#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters matched. Will resume with streaming" << std::endl;
#endif

		Run(); // Start thread loop

		return true;
	}
	else
	{
		m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_REFUSED_INVALID_PARAMETERS );
		m_pMessage->WriteMessage();

#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters mismatch detected. Will notify client and stop." << std::endl;
#endif

		return false;
	}
}

bool CITANetAudioStreamingServer::LoopBody()
{
	const double dNow = ITAClock::getDefaultClock()->getTime();

	if( m_dStreamTimeStart == 0.0f )
		m_dStreamTimeStart = dNow;

	CITAServerLog oLog;
	oLog.dWorldTimeStamp = dNow;
	oLog.uiBlockId = ++m_iServerBlockId;
	oLog.iTransmittedSamples = 0;

	// Sending Samples
	int iEstimatedClientRingBufferTargetLatencyFreeSamples = m_iEstimatedClientRingBufferFreeSamples - ( m_iClientRingBufferSize - m_iTargetLatencySamples );

	if( iEstimatedClientRingBufferTargetLatencyFreeSamples >= m_iSendingBlockLength )
	{
		// Send Samples
		int iSendBlocks = iEstimatedClientRingBufferTargetLatencyFreeSamples / m_iSendingBlockLength;

		// Besser wäre vermutlich, gleich alle samples zu senden und nicht nur einen Block nach dem anderen
		if( m_sfTempTransmitBuffer.GetLength() != m_iSendingBlockLength )
			m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), m_iSendingBlockLength, false );

		for( int j = 0; j < iSendBlocks; j++ )
		{
			for( int i = 0; i < int( m_pInputStream->GetNumberOfChannels() ); i++ )
			{
				ITAStreamInfo oStreamInfo;
				oStreamInfo.nSamples = ( m_nStreamSampleCounts += m_iSendingBlockLength );
				oStreamInfo.dTimecode = dNow - m_dStreamTimeStart;

				const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
				if( pfData != nullptr )
					m_sfTempTransmitBuffer[ i ].write( pfData, m_iSendingBlockLength, 0 );
			}

			m_pInputStream->IncrementBlockPointer();

			m_pMessage->ResetMessage();
			m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES );
			m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
			m_pMessage->WriteMessage();
			m_iEstimatedClientRingBufferFreeSamples -= m_iSendingBlockLength;
		}

#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Transmitted " << m_iSendingBlockLength << " samples for "
			<< m_pInputStream->GetNumberOfChannels() << " channels" << std::endl;
#endif

		oLog.iTransmittedSamples = iSendBlocks * m_pInputStream->GetBlocklength();
	}

	// Try-read incoming messages from client (e.g. regular status information)
	m_pMessage->ResetMessage();
	m_swTryReadBlockStats.start();
	m_swTryReadAccessStats.start();
	if( m_pMessage->ReadMessage( 1 ) )
	{
		m_swTryReadAccessStats.stop();
		int iMsgType = m_pMessage->GetMessageType();
		switch( iMsgType )
		{
		case CITANetAudioProtocol::NP_CLIENT_SENDING_RINGBUFFER_FREE_SAMPLES:
		{
			m_iEstimatedClientRingBufferFreeSamples = m_pMessage->ReadInt();
			m_dLastTimeStamp = dNow;
			break;
		}
		case CITANetAudioProtocol::NP_CLIENT_CLOSE:
		{
			// Stop here because answer on client close might be blocking, we don't want that in our statistics
			m_swTryReadBlockStats.stop();

			m_pMessage->ResetMessage();
			m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_CLOSE );
			m_pMessage->WriteMessage();

			StopGently( false );
			m_pConnection = NULL;
			Stop();
			break;
		}
		default:
		{
			vstr::out() << "[ ITANetAudioStreamingServer ] Unkown protocol type : " << iMsgType << std::endl;
			break;
		}
		}

		oLog.sProtocolStatus = CITANetAudioProtocol::GetNPMessageID( iMsgType );
	}
	else
	{
		// There is no status message, so we estimate the client-side ring buffer status
		const double dTimeDiff = dNow - m_dLastTimeStamp;
		m_dLastTimeStamp = dNow;
		double dEstimatedSamples = m_dEstimatedCorrFactor * dTimeDiff * m_pInputStream->GetSampleRate();
		m_iEstimatedClientRingBufferFreeSamples += ( int ) dEstimatedSamples;
		oLog.sProtocolStatus = "SERVER_ESTIMATION";
	}
	if( m_swTryReadBlockStats.started() ) // only stop if still running
		m_swTryReadBlockStats.stop();

	oLog.iEstimatedFreeSamples = m_iEstimatedClientRingBufferFreeSamples;
	oLog.dEstimatedCorrFactor = m_dEstimatedCorrFactor;
	m_pServerLogger->log( oLog );

	return false;
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if( VistaThreadLoop::IsRunning() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::GetNetStreamBlocklength() const
{
	return m_sfTempTransmitBuffer.GetLength();
}

int CITANetAudioStreamingServer::GetSendingBlockLength() const
{
	return m_iSendingBlockLength;
}

void CITANetAudioStreamingServer::SetSendingBlockLength( const int iSendingBlockLength )
{
	m_iSendingBlockLength = iSendingBlockLength;
}

int CITANetAudioStreamingServer::GetNetStreamNumberOfChannels() const
{
	return m_sfTempTransmitBuffer.channels();
}

void CITANetAudioStreamingServer::SetDebuggingEnabled( bool bEnabled )
{
	m_bDebuggingEnabled = bEnabled;
}

bool CITANetAudioStreamingServer::GetIsDebuggingEnabled() const
{
	return m_bDebuggingEnabled;
}

void CITANetAudioStreamingServer::SetTargetLatencySamples( const int iTargetLatency )
{
	// Streaming already set up?
	if( IsClientConnected() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Server not connected, client ring buffer unkown" );

	if( m_pInputStream )
		if ( iTargetLatency < int( m_pInputStream->GetBlocklength( ) ) )
			ITA_EXCEPT1( INVALID_PARAMETER, "Target latency has to be at least the block size of the audio streaming at client side." );

	m_iTargetLatencySamples = iTargetLatency;
}

void CITANetAudioStreamingServer::SetServerLogBaseName( const std::string& sBaseName )
{
	if( IsClientConnected() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming and logging already started." );

	assert( !m_sServerLogBaseName.empty() );
	m_sServerLogBaseName = sBaseName;
}

std::string CITANetAudioStreamingServer::GetServerLogBaseName() const
{
	return m_sServerLogBaseName;
}

bool CITANetAudioStreamingServer::IsClientConnected() const
{
	return m_pNetAudioServer->IsConnected();
}

std::string CITANetAudioStreamingServer::GetNetworkAddress() const
{
	return m_pNetAudioServer->GetServerAddress();
}

int CITANetAudioStreamingServer::GetNetworkPort() const
{
	return m_pNetAudioServer->GetNetworkPort();
}

double CITANetAudioStreamingServer::GetEstimatedCorrFactor( ) const
{
	return m_dEstimatedCorrFactor;
}

void CITANetAudioStreamingServer::SetEstimatedCorrFactor( double dCorrFactor )
{
	m_dEstimatedCorrFactor = dCorrFactor;
}



void CITANetAudioStreamingServer::Stop()
{
	m_pNetAudioServer->Stop();
	m_pMessage->ClearConnection();
}
