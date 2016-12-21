#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>
#include <ITANetAudioMessage.h>

// ITA includes
#include <ITADataSource.h>
#include <ITAException.h>
#include <ITAStreamInfo.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

// STL
#include <cmath>
#include <cassert>

CITANetAudioStreamingServer::CITANetAudioStreamingServer()
	: m_pInputStream( NULL )
	, m_iUpdateStrategy( AUTO )
	, m_pConnection( NULL )
{
	m_pNetAudioServer = new CITANetAudioServer();
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	// TODO: vorrückgabe noch anfangen zu senden (Samples)
	if( !m_pNetAudioServer->Start( sAddress, iPort ) ) // blocking
		return false;

	m_pConnection = m_pNetAudioServer->GetConnection();

	m_pMessage = new CITANetAudioMessage( m_pConnection->GetByteorderSwapFlag() );
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ReadMessage(); // blocking

	int nMT = m_pMessage->GetMessageType();
	assert( nMT == CITANetAudioProtocol::NP_CLIENT_OPEN );
	int i42 = m_pMessage->ReadInt();

	//CITANetAudioProtocol::StreamingParameters oClientParams = m_pMessage->ReadStreamingParameters();

	m_pMessage->SetAnswerType( CITANetAudioProtocol::NP_SERVER_OPEN );
	m_pMessage->WriteInt( 2 * 42 );
	m_pMessage->WriteAnswer();

	if( m_pInputStream )
		Run();

	return true;
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
}

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	if( VistaThreadLoop::IsRunning() )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Streaming loop already running, can not change input stream" );

	m_pInputStream = pInStream;
	m_sfTempTransmitBuffer.init( m_pInputStream->GetNumberOfChannels(), m_pInputStream->GetBlocklength(), true );

	if( m_pConnection )
		Run();
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

bool CITANetAudioStreamingServer::LoopBody()
{
	m_pMessage->ResetMessage();
	m_pMessage->SetConnection( m_pConnection );
	m_pMessage->ReadMessage(); // blocking

	switch( m_pMessage->GetMessageType() )
	{
	case CITANetAudioProtocol::NP_CLIENT_WAITING_FOR_SAMPLES:
		if( m_pInputStream )
		{
			for( int i = 0; i < m_pInputStream->GetNumberOfChannels(); i++ )
			{
				ITAStreamInfo oStreamInfo;
				const float* pfData = m_pInputStream->GetBlockPointer( i, &oStreamInfo );
				m_sfTempTransmitBuffer[ i ].write( pfData, m_pInputStream->GetBlocklength() );
			}
		}

		//m_pMessage->WriteSampleFrame( &m_sfTempTransmitBuffer );
		m_pMessage->WriteAnswer();
		break;
	}

	return true;
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::Transmit(const ITASampleFrame& sfNewSamples, int iNumSamples)
{
	return 0;
}
