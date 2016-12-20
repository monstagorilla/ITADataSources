#include <ITANetAudioServer.h>
#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioProtocol.h>

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

CITANetAudioServer::CITANetAudioServer(  )
	: m_pServer( NULL )
	, m_pSocket( NULL )
{
};

CITANetAudioServer::~CITANetAudioServer()
{
}

std::string CITANetAudioServer::GetServerAddress() const
{
	return m_sServerAddress;
}

int CITANetAudioServer::GetNetworkPort() const
{
	return m_iServerPort;
}

bool CITANetAudioServer::Start( const std::string& sAddress, int iPort )
{
	if( m_pServer )
		ITA_EXCEPT1( MODAL_EXCEPTION, "This net sample server is already started" );

	m_pServer = new VistaTCPServer( sAddress, iPort, 1 );
	m_sServerAddress = sAddress;
	m_iServerPort = iPort;

	// blocking wait for connection
	m_pSocket = m_pServer->GetNextClient();
	return true;
}

void CITANetAudioServer::Disconnect()
{
	m_pSocket = NULL;

	delete m_pServer;
	m_pServer = NULL;
}

bool CITANetAudioServer::IsConnected() const
{
	if( !m_pSocket )
		return false;

	return m_pSocket->GetIsConnected();
}


VistaTCPSocket* CITANetAudioServer::GetSocket() const
{
	return m_pSocket;
}