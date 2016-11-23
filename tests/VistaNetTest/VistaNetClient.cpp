#include <iostream>
#include <string>

#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

#include <ITAStringUtils.h>
#include <ITAStopWatch.h>
#include <ITAStringUtils.h>

int main(int argc, char** argv) {

	if( argc != 3 )
	{
		std::cout << "Syntax error: VistaNetClient SERVER PORT" << std::endl;
		return 255;
	}

	std::string sServerName = argv[1];
	int iServerPort = StringToInt( std::string( argv[2] ) );

	std::cout << "Attempting to connect to '" << sServerName << ":" << iServerPort << "'" << std::endl;

	ITAStopWatch sw; sw.start();
	VistaConnectionIP oCommandChannelConnection( VistaConnectionIP::CT_TCP, sServerName, iServerPort);

	if( oCommandChannelConnection.GetIsConnected() == false )
	{
		std::cerr << "Connection error, exiting." << std::endl;
		return 255;
	}

	std::string sLocalAddress;
	oCommandChannelConnection.GetLocalAddress().GetIPAddress().GetAddressString( sLocalAddress );
	
	int iBackchannelPort = 12481;
	VistaTCPServer oServer( sLocalAddress, iBackchannelPort, 1 );

	if( oServer.GetIsValid() == false)
	{
		std::cerr << "Could not start server" << std::endl;
		return 255;
	}

	// Transmit port on command channel
	oCommandChannelConnection.WriteRawBuffer( &iBackchannelPort, 8 );
	
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
