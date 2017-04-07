#include <cassert>
#include <cmath>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThread.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaInterProcComm/IPNet/VistaUDPSocket.h>
#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaTimeUtils.h>

using namespace std;

static const string g_sServerName = "localhost";
static const int g_iServerPort = 12480;
static const int g_iRepetitions = 5;
static const size_t g_lDataSize = 152733239; // 150 MBits
static const VistaConnectionIP::VistaProtocol g_iProtocol = VistaConnectionIP::CT_UDP;


class CUDPSocket : public VistaThread
{
public:
	inline CUDPSocket()
	{
		m_pUDPSocket = new VistaUDPSocket();
		if( !m_pUDPSocket->OpenSocket() )
		{
			vstr::out() << "[ UDPServer ] Could not open UDP server socket" << endl;
		}

		if( !m_pUDPSocket->BindToAddress( VistaSocketAddress( g_sServerName, g_iServerPort ) ) )
		{
			vstr::out() << "[ UDPServer ] Could not bind to UDP server addresss" << endl;
		}

		if( !m_pUDPSocket->ConnectToAddress( VistaSocketAddress( g_sServerName, g_iServerPort ) ) )
		{
			vstr::out() << "[ UDPServer ] Could not connect to UDP client" << endl;
		}

		Run();
	};

	inline ~CUDPSocket()
	{
		delete m_pUDPSocket;
	};

	inline void ThreadBody()
	{
		vstr::out() << "[ UDPServer ] Waiting for connections" << endl;

		while( !m_pUDPSocket->GetIsConnected() )
			VistaTimeUtils::Sleep( 100 );
		
		vstr::out() << "[ UDPServer ] Connected." << endl;
		
		size_t nGetReceiveBufferSize = m_pUDPSocket->GetReceiveBufferSize();
		vstr::out() << "[ UDPServer ] " << nGetReceiveBufferSize << " receive buffer size" << endl;

		vector< char > vdIncomingData;

		bool bRepeat = true;
		while( bRepeat )
		{
			long nIncomingBytes = m_pUDPSocket->WaitForIncomingData( 0 );
			vstr::out() << "[ UDPServer ] " << nIncomingBytes << " incoming bytes" << endl;

			// Two-step receiving
			int iPayloadDataSize;
			int iBytesRead = m_pUDPSocket->ReceiveRaw( &iPayloadDataSize, sizeof( int ) );
			assert( iBytesRead == sizeof( int ) );
			vstr::out() << "[ UDPServer ] Expecting " << iPayloadDataSize << " bytes to come." << endl;

			if( iPayloadDataSize > vdIncomingData.size() )
				vdIncomingData.resize( iPayloadDataSize );

			int iBytesReceivedTotal = 0;
			while( iPayloadDataSize != iBytesReceivedTotal )
			{
				long nIncomingBytes = m_pUDPSocket->WaitForIncomingData( 0 );
				int iBytesReceived = m_pUDPSocket->ReceiveRaw( &vdIncomingData[ iBytesReceivedTotal ], nIncomingBytes );
				iBytesReceivedTotal += iBytesReceived;
				vstr::out() << "[ UDPServer ] " << setw( 3 ) << std::floor( iBytesReceivedTotal / float( iPayloadDataSize ) * 100.0f ) << "% transmitted" << endl;
			}

			assert( vdIncomingData[ 0 ] == 1 );
			assert( vdIncomingData[ vdIncomingData.size() - 2 ] == -1 );

			vstr::out() << "[ UDPServer ] Received all data and content appears valid!" << endl;

			vstr::out() << "[ UDPServer ] Sending acknowledge flag" << endl;
			bool bTransmissionDone = true;
			m_pUDPSocket->SendRaw( &bTransmissionDone, sizeof( bool ) );
			iBytesRead = m_pUDPSocket->ReceiveRaw( &bRepeat, sizeof( bool ) );

			if( bRepeat )
				vstr::out() << "[ UDPServer ] Repeating" << endl;
		}

		vstr::out() << "[ UDPServer ] Closing." << endl;
	};

	VistaUDPSocket* m_pUDPSocket;
};

int main_udp()
{
	CUDPSocket oUDPSocket;

	VistaConnectionIP oConnection( VistaConnectionIP::CT_UDP, g_sServerName, g_iServerPort );

	if( !oConnection.Open() )
	{
		vstr::out() << "[ UDPClient ] Could not open UDP socket" << endl;
		return 255;
	}

	oConnection.Connect();
	
	while( !oConnection.GetIsConnected() )
		VistaTimeUtils::Sleep( 100 );

	vstr::out() << "[ UDPClient ] Connection established" << endl;

	vstr::out() << "[ UDPClient ] Connection is buffering: " << ( oConnection.GetIsBuffering() ? "yes" : "no" ) << endl;

	int i = 0;
	while( i++ <= g_iRepetitions )
	{
		vstr::out() << "[ UDPClient ] Client sending data now." << endl;
		vector< char > vdData( g_lDataSize + 4 );
		int* piDataSize = ( int* ) &vdData[ 0 ];
		*piDataSize = ( unsigned int ) ( g_lDataSize ); // Send data size as first block
		vdData[ 1 * sizeof( int ) + 0 ] = 1; // First entry one (just for fun)
		vdData[ vdData.size() - 2 ] = -1; // Second last entry -1 (just for fun)
		void* pData = ( void* ) &vdData[ 0 ];
		oConnection.Send( pData, int( vdData.size() ) );

		bool bAck;
		oConnection.ReadBool( bAck );

		if( !bAck )
			vstr::out() << "[ UDPClient ] Received negative acknowledge flag" << endl;
		else
			vstr::out() << "[ UDPClient ] Received positive acknowledge flag" << endl;

		bool bRepeat = ( i <= g_iRepetitions );
		oConnection.Send( &bRepeat, sizeof( bool ) );
	}

	oConnection.Close( false );

	return 0;
}

class CTCPServer : public VistaThread
{
public:
	inline CTCPServer()
	{
		m_pServer = new VistaTCPServer( g_sServerName, g_iServerPort, 1 );
		Run();
	};

	inline ~CTCPServer()
	{
		delete m_pServer;
	};

	inline void ThreadBody()
	{
		vstr::out() << "[ Server ] Waiting for connections" << endl;
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
				long nIncomingBytes = pSocket->WaitForIncomingData( 0 );
				int iBytesReceived = pSocket->ReceiveRaw( &vdIncomingData[ iBytesReceivedTotal ], nIncomingBytes );
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

int main_tcp()
{
	CTCPServer oServer;

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
		vector< char > vdData( g_lDataSize + 4 );
		int* piDataSize = ( int* ) &vdData[ 0 ];
		*piDataSize = (unsigned int)( g_lDataSize ); // Send data size as first block
		vdData[ 1 * sizeof( int ) + 0 ] = 1; // First entry one (just for fun)
		vdData[ vdData.size() - 2 ] = -1; // Second last entry -1 (just for fun)
		void* pData = ( void* ) &vdData[ 0 ];
		oConnection.Send( pData, int( vdData.size() ) );

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

int main( int, char** )
{
	if( g_iProtocol == VistaConnectionIP::CT_TCP )
		return main_tcp();
	if( g_iProtocol == VistaConnectionIP::CT_UDP )
		return main_udp();
}