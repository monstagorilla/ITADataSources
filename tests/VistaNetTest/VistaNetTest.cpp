#include <cassert>
#include <iostream>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaBase/VistaTimeUtils.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;


class CServer : public VistaThreadLoop
{
public:
	CServer()
	{
		m_pServer = new VistaTCPServer( g_sServerName, g_iServerPort, 1 );

		Run();
	}

	~CServer()
	{
		delete m_pServer;
	}

	bool LoopBody()
	{
		cout << "[ Server ] Waiting for connections" << endl;
		VistaTCPSocket* pSocket = m_pServer->GetNextClient();
		VistaTimeUtils::Sleep( 100 ); // Sync couts
		cout << "[ Server ] Connected." << endl;

		size_t nGetReceiveBufferSize = pSocket->GetReceiveBufferSize();
		cout << "[ Server ] " << nGetReceiveBufferSize << " receive buffer size" << endl;

		long nIncomingBytes = pSocket->WaitForIncomingData( 0 );
		cout << "[ Server ] " << nIncomingBytes << " incoming bytes" << endl;

		// Two-step receiving
		int iPayloadDataSize;
		int iIncomingBytes = pSocket->ReceiveRaw( &iPayloadDataSize, sizeof( int ) );
		cout << "[ Server ] Expecting " << iPayloadDataSize << " bytes to come." << endl;
		
		vector< char > vdIncomingData( iPayloadDataSize );
		int iBytesReceivedTotal = pSocket->ReceiveRaw( &vdIncomingData[ 0 ], iPayloadDataSize );
		cout << "[ Server ] Received " << iBytesReceivedTotal << " bytes, so far." << endl;

		while( iPayloadDataSize != iBytesReceivedTotal )
		{
			int iBytesReceived = pSocket->ReceiveRaw( &vdIncomingData[ iBytesReceivedTotal ], iPayloadDataSize );
			iBytesReceivedTotal += iBytesReceived;
			cout << "[ Server ] Further " << iBytesReceived << " bytes incoming" << endl;
		}
		
		assert( vdIncomingData[ 0 ] == 1 );
		assert( vdIncomingData[ vdIncomingData.size() - 2 ] == -1 );

		cout << "[ Server ] Received all data and content appears valid!" << endl;
		
		VistaTimeUtils::Sleep( 200 );

		cout << "[ Server ] Sending acknowledge flag" << endl;
		bool bAck = false;
		pSocket->SendRaw( &bAck, 1 );

		return false;
	}

private:
	VistaTCPServer* m_pServer;
};

int main( int , char** )
{

	CServer oServer;

	VistaTimeUtils::Sleep( 100 ); // Sync couts
	VistaConnectionIP oConnection( VistaConnectionIP::CT_TCP, g_sServerName, g_iServerPort);
	if( !oConnection.GetIsConnected() )
	{
		cerr << "[ Client ] Connection failed" << endl;
		return 255;
	}

	cout << "[ Client ] Connection established" << endl;
	VistaTimeUtils::Sleep( 500 );
	cout << "[ Client ] Client sending data" << endl;
	VistaTimeUtils::Sleep( 500 );

	cout << "[ Client ] Connection is buffering: " << ( oConnection.GetIsBuffering() ? "yes" : "no" ) << endl;

	size_t l = 1523633239; // > MTU?
	vector< char > vdData( l + 4 );
	int* piDataSize = ( int* ) &vdData[0];
	*piDataSize = unsigned int( l ); // Send data size as first block
	vdData[ 1 * sizeof( int ) + 0 ] = 1; // First entry one (just for fun)
	vdData[ vdData.size() - 2 ] = -1; // Second last entry -1 (just for fun)
	void* pData = (void*) &vdData[0];
	oConnection.Send( pData, int( vdData.size() ) ); // SendRaw?

	bool bAck;
	oConnection.ReadBool( bAck );

	VistaTimeUtils::Sleep( 100 ); // cout sync
	cout << "[ Client ] Received acknowledge flag '" << bAck << "', closing." << endl;

	oConnection.Close( false );

	return 0;
}
