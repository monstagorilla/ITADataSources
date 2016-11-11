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

	~CServer() {
		delete m_pServer;
	}

	bool LoopBody()
	{
		VistaTCPSocket* pSocket = m_pServer->GetNextClient();

		VistaTimeUtils::Sleep( 100 ); // Sync couts

		size_t nGetReceiveBufferSize = pSocket->GetReceiveBufferSize();

		long nIncomingBytes = pSocket->WaitForIncomingData( 0 );
		cout << "Server incoming bytes: " << nIncomingBytes << " bytes" << endl;

		if( nGetReceiveBufferSize == nIncomingBytes )
			cout << "Warning: payload as long as receiver buffer size ... problem will occur!" << endl;
		
		char* buf = new char[ nIncomingBytes ];

		long nIncomingBytes2 = pSocket->WaitForIncomingData( 0 );
		int iBytesReceived = pSocket->ReceiveRaw( buf, nIncomingBytes );
		
		vector< char > vdIncomingData( iBytesReceived );
		for( size_t i=0; i < vdIncomingData.size(); i++ )
			vdIncomingData[i] = buf[ i ];

		delete[] buf;	

		assert( vdIncomingData[0] == 1 );
		assert( vdIncomingData[ vdIncomingData.size() -2 ] == -1 );

		VistaTimeUtils::Sleep( 200 );

		cout << "Server sending acknowledge flag" << endl;
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

	VistaConnectionIP oConnection( VistaConnectionIP::CT_TCP, g_sServerName, g_iServerPort);
	if( oConnection.GetIsConnected() )
	{
		cout << "Connection established" << endl;
		VistaTimeUtils::Sleep( 500 );
		cout << "Client sending data" << endl;
		VistaTimeUtils::Sleep( 500 );

		cout << "Connection is buffering: " << ( oConnection.GetIsBuffering() ? "yes" : "no" ) << endl;

		size_t l = 10000; // > MTU?
		vector< char > vdData( l );
		vdData[ 0 ] = 1; // First entry one
		vdData[ vdData.size() - 2 ] = -1; // Last entry -1
		void* pData = (void*) &vdData[0];
		oConnection.Send( pData, int( vdData.size() ) ); // SendRaw?

		bool bAck;
		oConnection.ReadBool( bAck );
		cout << "Client received acknowledge flag '" << bAck << "'" << endl;

		oConnection.Close( false );
	}
	else
	{
		cerr << "Connection failed" << endl;
		return 255;
	}

	return 0;
}
