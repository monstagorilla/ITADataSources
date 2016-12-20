#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioServer.h>

// ITA includes
#include <ITADataSource.h>
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
{
	m_pNetAudioServer = new CITANetAudioServer( this );
}

bool CITANetAudioStreamingServer::Start( const std::string& sAddress, int iPort )
{
	return m_pNetAudioServer->Start( sAddress, iPort );
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

void CITANetAudioStreamingServer::SetInputStream( ITADatasource* pInStream )
{
	m_pInputStream = pInStream;
}

void CITANetAudioStreamingServer::SetAutomaticUpdateRate()
{
	m_iUpdateStrategy = AUTO;
}

ITADatasource* CITANetAudioStreamingServer::GetInputStream() const
{
	return m_pInputStream;
}
