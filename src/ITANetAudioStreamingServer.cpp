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
#include <VistaInterProcComm/Concurrency/VistaPriority.h>
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
		os << "\t" << "EstimatedFreeSamples";
		os << "\t" << "TransmittedSamples";
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
		os << std::endl;
		return os;
	};

	unsigned int uiBlockId; //!< Block identifier (audio streaming)
	double dWorldTimeStamp;
	std::string sProtocolStatus;
	int iEstimatedFreeSamples;
	int iTransmittedSamples;
};

class ITABufferedDataLoggerImplServer : public ITABufferedDataLogger < ITAServerLog > {};

CITANetAudioStreamingServer::CITANetAudioStreamingServer()
	: m_pInputStream( NULL )
	, m_iUpdateStrategy( AUTO )
	, m_pConnection( NULL )
	, m_pNetAudioServer( new CITANetAudioServer() )
	, m_dLastTimeStamp( 0 )
	, m_iTargetLatencySamples( -1 )
	, m_sServerLogBaseName( "ITANetAudioStreamingServer" )
{
	iServerBlockId = 0;
	m_iMaxSendBlocks = 40;
	m_iEstimatedClientRingBufferFreeSamples = 0;

	VistaThread::SetPriority( VistaPriority::VISTA_MID_PRIORITY );
}

CITANetAudioStreamingServer::~CITANetAudioStreamingServer()
{
	delete m_pNetAudioServer;
	delete m_pServerLogger;

	vstr::out() << "[ ITANetAudioStreamingServer ] Processing statistics: " << m_swTryReadBlockStats.ToString() << std::endl;
	vstr::out() << "[ ITANetAudioStreamingServer ] Try-read access statistics: " << m_swTryReadAccessStats.ToString() << std::endl;
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort, double dTimeIntervalCientSendStatus )
{
	if( !m_pInputStream )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not start server without a valid input stream" );

	if( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection();

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag() );
	m_pMessage->SetMessageLoggerBaseName( GetServerLogBaseName() + "_Messages" );

	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	while( !m_pMessage->ReadMessage( 0 ) ); //blocking

	assert( m_pMessage->GetMessageType() == CITANetAudioProtocol::NP_CLIENT_OPEN );
	CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters();

	bool bOK = false;
	m_oServerParams.iRingBufferSize = oClientParams.iRingBufferSize;
	m_oServerParams.iBlockSize = oClientParams.iBlockSize;
	m_iEstimatedClientRingBufferFreeSamples = m_oServerParams.iRingBufferSize;

	m_dLastTimeStamp = ITAClock::getDefaultClock()->getTime();
	if( m_oServerParams == oClientParams )
	{
		bOK = true;
#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters matched. Will resume with streaming" << std::endl;
#endif
	}
	else
	{
#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Server and client parameters mismatch detected. Will notify client and stop." << std::endl;
#endif
}

	m_pServerLogger = new ITABufferedDataLoggerImplServer();
	m_pServerLogger->setOutputFile( m_sServerLogBaseName + ".log" );

	m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_OPEN );
	m_pMessage->WriteDouble( dTimeIntervalCientSendStatus );
	m_pMessage->WriteMessage();

	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), m_oServerParams.iRingBufferSize, true );


	if( bOK )
		Run();

	return bOK;
}

bool CITANetAudioStreamingServer::LoopBody()
{
	const double dNow = ITAClock::getDefaultClock()->getTime();

	ITAServerLog oLog;
	oLog.dWorldTimeStamp = dNow;
	oLog.uiBlockId = ++iServerBlockId;
	oLog.iTransmittedSamples = 0;
	
	// Sending Samples
	int iBlockLength = m_pInputStream->GetBlocklength();
	int iEstimatedClientRingBufferTargetLatencyFreeSamples = m_iEstimatedClientRingBufferFreeSamples - ( m_oServerParams.iRingBufferSize - m_iTargetLatencySamples );

	if( iEstimatedClientRingBufferTargetLatencyFreeSamples >= iBlockLength )
	{
		// Send Samples
		int iSendBlocks = iEstimatedClientRingBufferTargetLatencyFreeSamples / iBlockLength;

		// Besser wäre vermutlich, gleich alle samples zu senden und nicht nur einen Block nach dem anderen
		if( m_sfTempTransmitBuffer.GetLength() != iBlockLength )
			m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), iBlockLength, false );

		for( int j = 0; j < iSendBlocks; j++ )
		{
			for( int i = 0; i < int( m_pInputStream->GetNumberOfChannels() ); i++ )
			{
				ITAStreamInfo oStreamInfo;
				oStreamInfo.nSamples = iBlockLength;

				const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
				if( pfData != 0 )
					m_sfTempTransmitBuffer[ i ].write( pfData, iBlockLength, 0 );
			}

			m_pInputStream->IncrementBlockPointer();

			m_pMessage->ResetMessage();
			m_pMessage->SetMessageType( CITANetAudioProtocol::NP_SERVER_SENDING_SAMPLES );
			m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
			m_pMessage->WriteMessage();
			m_iEstimatedClientRingBufferFreeSamples -= iBlockLength;
		}

#ifdef NET_AUDIO_SHOW_TRAFFIC
		vstr::out() << "[ITANetAudioStreamingServer] Transmitted " << iSendSamples << " samples for "
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
		double dEstimatedSamples = dTimeDiff * m_pInputStream->GetSampleRate();
		m_iEstimatedClientRingBufferFreeSamples += ( int ) dEstimatedSamples;
		oLog.sProtocolStatus = "SERVER_ESTIMATION";
	}
	if( m_swTryReadBlockStats.started() ) // only stop if still running
		m_swTryReadBlockStats.stop();
	
	oLog.iEstimatedFreeSamples = m_iEstimatedClientRingBufferFreeSamples;
	m_pServerLogger->log( oLog );

	return false;
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if( VistaThreadLoop::IsRunning() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_oServerParams.dSampleRate = m_pInputStream->GetSampleRate();
	m_oServerParams.iBlockSize = m_pInputStream->GetBlocklength();
	m_oServerParams.iChannels = m_pInputStream->GetNumberOfChannels();
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::GetNetStreamBlocklength() const
{
	return m_sfTempTransmitBuffer.GetLength();
}

int CITANetAudioStreamingServer::GetNetStreamNumberOfChannels() const
{
	return m_sfTempTransmitBuffer.channels();
}

void CITANetAudioStreamingServer::SetAutomaticUpdateRate()
{
	m_iUpdateStrategy = AUTO;
}

void CITANetAudioStreamingServer::SetTargetLatencySamples( const int iTargetLatency )
{
	// Streaming already set up?
	if( IsClientConnected() && m_iTargetLatencySamples < m_oServerParams.iBlockSize )
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

void CITANetAudioStreamingServer::Stop()
{
	m_pNetAudioServer->Stop();
	m_pMessage->ClearConnection();
}
