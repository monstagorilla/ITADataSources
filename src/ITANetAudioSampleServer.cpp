#include <ITANetAudioSampleServer.h>
#include <ITANetAudioStreamingServer.h>

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

CITANetAudioSampleServer::CITANetAudioSampleServer()
	: m_pInputStream( NULL )
	, m_iUpdateStrategy( AUTO )
{
	m_pNetAudioServer = new CITANetAudioStreamServer( this );
}

bool CITANetAudioSampleServer::Start( const std::string& sAddress, int iPort )
{
	return m_pNetAudioServer->Start( sAddress, iPort );
}

bool CITANetAudioSampleServer::IsClientConnected() const
{
	return m_pNetAudioServer->IsConnected();
}

std::string CITANetAudioSampleServer::GetNetworkAddress() const
{
	return m_pNetAudioServer->GetServerAddress();
}

int CITANetAudioSampleServer::GetNetworkPort() const
{
	return m_pNetAudioServer->GetNetworkPort();
}

void CITANetAudioSampleServer::SetInputStream( ITADatasource* pInStream )
{
	m_pInputStream = pInStream;
}

void CITANetAudioSampleServer::SetAutomaticUpdateRate()
{
	m_iUpdateStrategy = AUTO;
}

ITADatasource* CITANetAudioSampleServer::GetInputStream() const
{
	return m_pInputStream;
}
