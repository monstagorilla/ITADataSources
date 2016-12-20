#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>

// ITA includes
#include <ITADataSource.h>
#include <ITANetAudioMessage.h>
#include <ITAException.h>
#include <ITAStreamInfo.h>

// Vista includes
#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
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
	// TODO: Init members
}

bool CITANetAudioStreamingServer::Start(const std::string& sAddress, int iPort)
{
	// TODO: vorrückgabe noch anfangen zu senden (Samples)
	if (!m_pNetAudioServer->Start(sAddress, iPort))
		return false;

	m_pConnection = m_pNetAudioServer->GetConnetion();
	m_pMessage->SetConnection( m_pConnection );

	// Get Streaming Parameters from Client
	m_pMessage->ReadAnswer();
	CITANetAudioProtocol::StreamingParameters m_oServerParams = m_pMessage->ReadStreamingParameters();
	
	// Send Streaming Parameters from Client back
	m_pMessage->ResetMessage();
	m_pMessage->SetMessageType(CITANetAudioProtocol::NP_SERVER_OPEN);
	m_pMessage->WriteStreamingParameters(m_oServerParams);
	m_pMessage->WriteMessage();


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
bool CITANetAudioStreamingServer::LoopBody()
{
	return true;
}

void CITANetAudioStreamingServer::Stop() 
{
	m_pNetAudioServer->Disconnect();
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

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}

int CITANetAudioStreamingServer::Transmit(const ITASampleFrame& sfNewSamples, int iNumSamples)
{
	return 0;
}
