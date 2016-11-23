#include <iostream>
#include <string>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

#include <ITAStringUtils.h>
#include <ITAStopwatch.h>
#include <ITAStringUtils.h>

int main(int argc, char** argv)
{
	std::string sServerName ="localhost";
	int iServerPort = 12343;

	std::cout << "Attempting to connect to ITANA server at '" << sServerName << ":" << iServerPort << "'" << std::endl;

	VistaConnectionIP oCommandChannelConnection( VistaConnectionIP::CT_TCP, sServerName, iServerPort);

	if( oCommandChannelConnection.GetIsConnected() == false )
	{
		std::cerr << "Connection error, exiting." << std::endl;
		return 255;
	}

	std::string sLocalAddress;
	oCommandChannelConnection.GetLocalAddress().GetIPAddress().GetAddressString( sLocalAddress );
	
	int iBackChannelPort = 12481;
	VistaTCPServer oServer( sLocalAddress, iBackChannelPort, 1 );

	if( oServer.GetIsValid() == false)
	{
		std::cerr << "Could not start server" << std::endl;
		return 255;
	}
	else
	{
		std::cout << "Duplex connection opened, data communication channel port: " << iBackChannelPort << std::endl;
	}

	// Transmit port on command channel
	oCommandChannelConnection.WriteRawBuffer( &iBackChannelPort, 8 );
	
	VistaTCPSocket* pSocket = oServer.GetNextClient();
	unsigned long l = pSocket->WaitForIncomingData( 0 );

	if( l == sizeof( VistaType::byte ) )
	{
		bool bAck;
		pSocket->ReceiveRaw( &bAck, l );
		std::cout << "Client received acknowledge flag '" << bAck << "'" << std::endl;
	}

	oCommandChannelConnection.Close( false );
	
	return 0;
}
