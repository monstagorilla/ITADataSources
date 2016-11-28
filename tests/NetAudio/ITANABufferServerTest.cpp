#include <iostream>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
//#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

#include <ITASampleFrame.h>

#include <cassert>


using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;

//! NetAudio buffer server
class CITANAServer : public VistaThreadLoop
{
public:
	CITANAServer()
	{
		m_pServer = new VistaTCPServer( g_sServerName, g_iServerPort, 1 );
		cout << "ITANA server running, waiting for connection" << endl;

		m_sfBuffer.Load( "Bauer.wav" );
		m_iReadPointer = 0;

		m_pSocket = m_pServer->GetNextClient();

		Run();
	}

	~CITANAServer()
	{
		delete m_pServer;
	}

	bool LoopBody()
	{
		if( m_pSocket->GetIsConnected() == false )
		{
			cout << "Connection closed. Stopping loop." << endl;
			StopGently( true );

			return false;
		}

		size_t nGetReceiveBufferSize = m_pSocket->GetReceiveBufferSize();

		cout << "Waiting for new incoming data" << endl;
		long nIncomingBytes = m_pSocket->WaitForIncomingData( 0 );
		cout << "Server incoming bytes: " << nIncomingBytes << " bytes" << endl;

		if( nGetReceiveBufferSize == nIncomingBytes )
			cout << "Warning: payload as long as receiver buffer size ... problem will occur!" << endl;

		if( nIncomingBytes == 0 )
		{
			cout << "Connection closed. Stopping loop." << endl;
			StopGently( true );

			return false;
		}

		assert( sizeof( int ) <= nIncomingBytes );
		int iChannelIndex;
		int iBytesReceived = m_pSocket->ReceiveRaw( &iChannelIndex, sizeof( int ) );

		nIncomingBytes = m_pSocket->WaitForIncomingData( 0 );
		assert( sizeof( int ) <= nIncomingBytes );
		int iNumSamples;
		iBytesReceived = m_pSocket->ReceiveRaw( &iNumSamples, sizeof( int ) );

		if( m_sfOutBuffer.channels() <= iChannelIndex || m_sfOutBuffer.length() != iNumSamples )
			m_sfOutBuffer.init( iChannelIndex + 1, iNumSamples, true );
		else
			m_sfOutBuffer.zero();

		cout << "Server sending samples";
		ITASampleBuffer* pBuf = &m_sfBuffer[ iChannelIndex ];
		pBuf->cyclic_read( m_sfOutBuffer[ iChannelIndex ].data(), iNumSamples, m_iReadPointer );
		m_pSocket->SendRaw( m_sfOutBuffer[ iChannelIndex ].data(), iNumSamples * sizeof( float ) );
		m_pSocket->WaitForSendFinish( 0 );
		cout << " ... finished" << endl;

		m_iReadPointer += iNumSamples;
		m_iReadPointer = m_iReadPointer % m_sfBuffer.length();

		return true;
	}

private:
	VistaTCPServer* m_pServer;
	VistaTCPSocket* m_pSocket;
	ITASampleFrame m_sfBuffer;
	ITASampleFrame m_sfOutBuffer;
	int m_iReadPointer;
};

int main( int , char**  )
{
	CITANAServer oITANAServer;
	
	cout << "Press any key to quit" << endl;

	int iIn;
	cin >> iIn;

	return 0;
}
