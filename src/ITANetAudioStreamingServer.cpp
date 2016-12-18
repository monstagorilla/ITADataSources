#include <ITANetAudioStreamingServer.h>
#include <ITANetAudioSampleServer.h>
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

CITANetAudioStreamServer::CITANetAudioStreamServer( CITANetAudioSampleServer* pParent )
	: m_pParent( pParent )
	, m_bStopIndicated( false )
	, m_pServer( NULL )
	, m_pSocket( NULL )
	, m_iClientRingBufferSize( -1 )
	, m_iClientBufferSize( -1 )
	, m_iClientRingBufferFreeSamples( 0 )
	, m_dClientSampleRate( -1 )
	, m_iServerPort( -1 )
{
};

CITANetAudioStreamServer::~CITANetAudioStreamServer()
{
}

std::string CITANetAudioStreamServer::GetServerAddress() const
{
	return m_sServerAddress;
}

int CITANetAudioStreamServer::GetNetworkPort() const
{
	return m_iServerPort;
}

bool CITANetAudioStreamServer::Start( const std::string& sAddress, int iPort )
{
	if( m_pServer )
		ITA_EXCEPT1( MODAL_EXCEPTION, "This net sample server is already started" );

	m_pServer = new VistaTCPServer( sAddress, iPort, 1 );
	m_sServerAddress = sAddress;
	m_iServerPort = iPort;

	m_pSocket = m_pServer->GetNextClient();

	long nIncomingBytes = m_pSocket->WaitForIncomingData( 0 );
	int iBytesReceived = m_pSocket->ReceiveRaw( &m_iClientChannels, sizeof( int ) );
	iBytesReceived = m_pSocket->ReceiveRaw( &m_dClientSampleRate, sizeof( double ) );
	iBytesReceived = m_pSocket->ReceiveRaw( &m_iClientBufferSize, sizeof( int ) );
	iBytesReceived = m_pSocket->ReceiveRaw( &m_iClientRingBufferSize, sizeof( int ) );
	m_iClientRingBufferFreeSamples = m_iClientRingBufferFreeSamples;

	int iMessageID = 1;
	m_pSocket->SendRaw( &iMessageID, sizeof( int ) );

	Run();
}

void CITANetAudioStreamServer::Disconnect()
{
	m_bStopIndicated = true;
	StopGently( true );

	m_pSocket = NULL;

	delete m_pServer;
	m_pServer = NULL;

	m_bStopIndicated = false;
}

bool CITANetAudioStreamServer::IsConnected() const
{
	if( !m_pSocket )
		return false;

	return m_pSocket->GetIsConnected();
}

bool CITANetAudioStreamServer::LoopBody()
{
	if( m_bStopIndicated )
		return true;

	if( m_pSocket->GetIsConnected() == false )
	{
		StopGently( true );
		return false;
	}

	ITAStreamInfo oStreamInfo;

	ITADatasource* pIn = m_pParent->GetInputStream();
	for( int iChannelIndex = 0; iChannelIndex < int( m_pParent->GetInputStream()->GetNumberOfChannels() ); iChannelIndex++ )
	{
		const float* pfData = pIn->GetBlockPointer( iChannelIndex, &oStreamInfo );
		int iNumSamples = pIn->GetBlocklength();
		m_pSocket->SendRaw( pfData, iNumSamples * sizeof( float ) );
	}

	return true;
}