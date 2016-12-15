#include <ITANetAudioSampleServer.h>

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

class CITANetAudioStreamServer : public VistaThreadLoop
{
public:
	enum MessageType
	{
		NET_MESSAGE_NONE = 0,
		NET_MESSAGE_OPEN,
		NET_MESSAGE_CLOSE,
		NET_MESSAGE_SAMPLES,
	};

	inline CITANetAudioStreamServer( CITANetAudioSampleServer* pParent )
		: m_pParent( pParent )
		, m_bStopIndicated( false )
		, m_pServer( NULL )
		, m_pSocket( NULL )
		, m_iClientRingBufferSize( - 1 )
		, m_iClientBufferSize( -1 )
		, m_iClientRingBufferFreeSamples( 0 )
		, m_dClientSampleRate( -1 )
	{
	};

	inline ~CITANetAudioStreamServer()
	{
	};

	inline std::string GetServerAddress() const
	{
		if( m_pServer )
		{
			std::string sAddress;
			m_pServer->GetServerAddress().GetIPAddress().GetAddressString( sAddress );
			return sAddress;
		}
		else
			return "";
	};

	inline int GetNetworkPort() const
	{
		if( m_pServer )
			m_pServer->GetServerAddress().GetPortNumber();
		else
			return -1;
	};

	inline bool Start( const std::string& sAddress, int iPort )
	{
		if( m_pServer )
			ITA_EXCEPT1( MODAL_EXCEPTION, "This net sample server is already started" );

		m_pServer = new VistaTCPServer( sAddress, iPort, 1 );

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
	};

	inline void Disconnect()
	{
		m_bStopIndicated = true;
		StopGently( true );

		m_pSocket = NULL;

		delete m_pServer;
		m_pServer = NULL;

		m_bStopIndicated = false;
	};

	inline bool IsConnected() const
	{
		if( !m_pSocket )
			return false;

		return m_pSocket->GetIsConnected();
	};

	inline bool LoopBody()
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
	};

private:
	VistaTCPServer* m_pServer;
	VistaTCPSocket* m_pSocket;
	CITANetAudioSampleServer* m_pParent;
	ITASampleFrame m_sfReceivingBuffer;
	
	bool m_bStopIndicated;
	
	int m_iClientChannels;
	int m_iClientRingBufferSize;
	int m_iClientBufferSize;
	int m_iClientRingBufferFreeSamples;
	double m_dClientSampleRate;
};

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
