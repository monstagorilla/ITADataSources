#include <iostream>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
//#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

#include <ITAStringUtils.h>

int main( int argc, char** argv )
{
	std::string sServerName = "localhost";
	int iServerPort = 12343;

	std::cout << "Setting up ITANA server command channel on '" << sServerName << ":" << iServerPort << "'" << std::endl;
	VistaTCPServer oNAServer( sServerName, iServerPort, 1 );

	std::cout << "Waiting for connection" << std::endl;
	VistaTCPSocket* pSocket = oNAServer.GetNextClient();

	if( oNAServer.GetIsValid() == false )
	{
		std::cerr << "Could not start server" << std::endl;
		return 255;
	}

	std::cout << "Waiting for result channel port" << std::endl;
	unsigned long l = pSocket->WaitForIncomingData( 0 );

	if( l == 8 )
	{
		unsigned long iRequestedResultChannelPort;
		pSocket->ReceiveRaw( &iRequestedResultChannelPort, 8 );

		std::string sRemoteAddress;
		VistaSocketAddress sAddr;
		pSocket->GetPeerSockName( sAddr );
		sAddr.GetIPAddress().GetAddressString( sRemoteAddress );

		VistaConnectionIP oConnection( VistaConnectionIP::CT_TCP, sRemoteAddress, iRequestedResultChannelPort );

		bool bAck = oConnection.GetIsConnected();
		oConnection.Send( &bAck, sizeof( bool ) );
		oConnection.WaitForSendFinish( 0 );

		std::cout << "Result channel connection successfully established" << std::endl;

		oConnection.Close( false );
	}

	pSocket->CloseSocket();

	return 0;
}
