#include "ITANetAudioServer.h"
#include "ITANetAudioProtocol.h"

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
#include <VistaInterProcComm/IPNet/VistaSocketAddress.h>
#include <VistaInterProcComm/IPNet/VistaUDPSocket.h>
#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

// STL
#include <cmath>
#include <cassert>

CITANetAudioServer::CITANetAudioServer()
	: m_pTCPServer( NULL )
	, m_pTCPSocket( NULL )
	, m_pUDPSocket( NULL )
	, m_pConnection( NULL )
	, m_iServerPort( -1 )
{
};

CITANetAudioServer::~CITANetAudioServer()
{
	m_pTCPSocket = NULL;

	delete m_pUDPSocket;
	m_pUDPSocket = NULL;

	delete m_pConnection;
	delete m_pTCPServer;
}

std::string CITANetAudioServer::GetServerAddress() const
{
	return m_sServerAddress;
}

int CITANetAudioServer::GetNetworkPort() const
{
	return m_iServerPort;
}

bool CITANetAudioServer::Start( const std::string& sAddress, const int iPort, const bool bUseUDP )
{
	if( m_pTCPServer || m_pUDPSocket )
		ITA_EXCEPT1( MODAL_EXCEPTION, "This NetAudio server is already started" );

	m_sServerAddress = sAddress;
	m_iServerPort = iPort;
	
	if( bUseUDP )
	{
		VistaSocketAddress oAddress( sAddress, iPort );
		VistaUDPSocket* pUDPSocket = new VistaUDPSocket();

		// blocking wait for connection
		pUDPSocket->ConnectToAddress( oAddress );

		if( pUDPSocket->GetIsConnected() )
			m_pConnection = new VistaConnectionIP( pUDPSocket );
	}
	else
	{
		m_pTCPServer = new VistaTCPServer( sAddress, iPort, 1 );

		// blocking wait for connection
		m_pTCPSocket = m_pTCPServer->GetNextClient();
		if( !m_pTCPSocket )
			return false;

		if( m_pTCPSocket->GetIsConnected() )
			m_pConnection = new VistaConnectionIP( m_pTCPSocket );
	}

	if( !m_pConnection )
		return false;

	return true;
}

VistaConnectionIP* CITANetAudioServer::GetConnection() const
{
	return m_pConnection;
}


void CITANetAudioServer::Stop()
{
	delete m_pConnection;
	m_pConnection = NULL;

	m_pTCPSocket = NULL;

	delete m_pTCPServer;
	m_pTCPServer = NULL;

	delete m_pUDPSocket;
	m_pUDPSocket = NULL;
}

bool CITANetAudioServer::IsConnected() const
{
	return m_pConnection ? true : false;
}
