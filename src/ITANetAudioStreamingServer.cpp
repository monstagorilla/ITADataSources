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
	// TODO: vorr�ckgabe noch anfangen zu senden (Samples)
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
	m_pInputStream = pInStream;
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
