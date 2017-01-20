#include <cassert>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThread.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaBase/VistaStreamUtils.h>

using namespace std;

static string g_sServerName = "localhost";
static int g_iServerPort = 12480;
static int g_iRepetitions = 10;


class CServer : public VistaThread
{
public:
	CServer()
	{
		m_pServer = new VistaTCPServer( g_sServerName, g_iServerPort, 1 );
		Run();
	};

	~CServer()
	{
		delete m_pServer;
	};

	void ThreadBody()
	{
		cout << "[ Server ] Waiting for connections" << endl;
		VistaTCPSocket* pSocket = m_pServer->GetNextClient();
		vstr::out() << "[ Server ] Connected." << endl;

		size_t nGetReceiveBufferSize = pSocket->GetReceiveBufferSize();
		vstr::out() << "[ Server ] " << nGetReceiveBufferSize << " receive buffer size" << endl;

		vector< char > vdIncomingData;

		bool bRepeat = true;
		while( bRepeat )
		{
			long nIncomingBytes = pSocket->WaitForIncomingData( 0 );
			vstr::out() << "[ Server ] " << nIncomingBytes << " incoming bytes" << endl;

			// Two-step receiving
			int iPayloadDataSize;
			int iBytesRead = pSocket->ReceiveRaw( &iPayloadDataSize, sizeof( int ) );
			assert( iBytesRead == sizeof( int ) );
			vstr::out() << "[ Server ] Expecting " << iPayloadDataSize << " bytes to come." << endl;

			if( iPayloadDataSize > vdIncomingData.size() )
				vdIncomingData.resize( iPayloadDataSize );

			int iBytesReceivedTotal = 0;
			while( iPayloadDataSize != iBytesReceivedTotal )
			{
				int iBytesReceived = pSocket->ReceiveRaw( &vdIncomingData[ iBytesReceivedTotal ], iPayloadDataSize );
				iBytesReceivedTotal += iBytesReceived;
				vstr::out() << "[ Server ] " << setw( 3 ) << std::floor( iBytesReceivedTotal / float( iPayloadDataSize ) * 100.0f ) << "% transmitted" << endl;
			}

			assert( vdIncomingData[ 0 ] == 1 );
			assert( vdIncomingData[ vdIncomingData.size() - 2 ] == -1 );

			vstr::out() << "[ Server ] Received all data and content appears valid!" << endl;

			vstr::out() << "[ Server ] Sending acknowledge flag" << endl;
			bool bTransmissionDone = true;
			pSocket->SendRaw( &bTransmissionDone, sizeof( bool ) );
			iBytesRead = pSocket->ReceiveRaw( &bRepeat, sizeof( bool ) );

			if( bRepeat )
				vstr::out() << "[ Server ] Repeating" << endl;
		}

		vstr::out() << "[ Server ] Closing." << endl;
	};

private:
	VistaTCPServer* m_pServer;
};

int main( int , char** )
{
	CServer oServer;

	VistaConnectionIP oConnection( VistaConnectionIP::CT_TCP, g_sServerName, g_iServerPort);
	if( !oConnection.GetIsConnected() )
	{
		vstr::out() << "[ Client ] Connection failed" << endl;
		return 255;
	}

	vstr::out() << "[ Client ] Connection established" << endl;
	vstr::out() << "[ Client ] Connection is buffering: " << ( oConnection.GetIsBuffering() ? "yes" : "no" ) << endl;

	int i = 0;
	while( i++ <= g_iRepetitions )
	{
		vstr::out() << "[ Client ] Client sending data now." << endl;
		size_t l = 1523633239; // > MTU?
		vector< char > vdData( l + 4 );
		int* piDataSize = ( int* ) &vdData[ 0 ];
		*piDataSize = unsigned int( l ); // Send data size as first block
		vdData[ 1 * sizeof( int ) + 0 ] = 1; // First entry one (just for fun)
		vdData[ vdData.size() - 2 ] = -1; // Second last entry -1 (just for fun)
		void* pData = ( void* ) &vdData[ 0 ];
		oConnection.Send( pData, int( vdData.size() ) ); // SendRaw?

		bool bAck;
		oConnection.ReadBool( bAck );

		if( !bAck )
			vstr::out() << "[ Client ] Received negative acknowledge flag" << endl;
		else
			vstr::out() << "[ Client ] Received positive acknowledge flag" << endl;

		bool bRepeat = ( i <= g_iRepetitions );
		oConnection.Send( &bRepeat, sizeof( bool ) );
	}

	oConnection.Close( false );

	return 0;
}
