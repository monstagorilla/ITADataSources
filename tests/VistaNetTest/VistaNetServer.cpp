#include <iostream>
#include <string>

#include <VistaInterProcComm/Concurrency/VistaThreadLoop.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
//#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>

#include <ITAStringUtils.h>
#include <ITASampleFrame.h>
#include <ITAAudiofileWriter.h>


int main(int argc, char** argv) {

	if( argc != 3 )
	{
		std::cerr << "Syntax error: VistaNetServer SERVER PORT" << std::endl;
		return 255;
	}

	std::string sServerName = argv[1];
	int iServerPort = StringToInt( std::string( argv[2] ) );

	std::cout << "Setting up server command channel on '" << sServerName << ":" << iServerPort << "'" << std::endl;
	VistaTCPServer oServer( sServerName, iServerPort, 1 );

	std::cout << "Waiting for connection" << std::endl;
	VistaTCPSocket* pSocket = oServer.GetNextClient();

	if( oServer.GetIsValid() == false)
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

		VistaConnectionIP oConnection( VistaConnectionIP::CT_TCP, sRemoteAddress, iRequestedResultChannelPort);

		// Bool senden
		//bool bAck = oConnection.GetIsConnected();
		//oConnection.Send(&bAck, sizeof( bool ) );

		// Bool Array Senden
		//bool bArr[5] = { 1, 0, 1, 0 };
		//oConnection.Send(&bArr, sizeof(bArr));

		// int array senden
		//int iArr[6] = { 12, 2, 34, -5, 111 };

		//oConnection.Send(&iArr, sizeof(iArr));


		//einlesen waveDatei
		ITASampleFrame oMusik;
		oMusik.Load("music.wav");
		int iLength = oMusik.GetLength();
		int iChannels = oMusik.channels();
		int iFrequency = 44100;
		writeAudiofile("sendMusik.wav", &oMusik, 44100, ITAQuantization::ITA_INT16);

		//senden der infos vor übertragung
		int iDetails[3] = { iLength, iChannels, iFrequency};
		oConnection.Send(&iDetails, sizeof(iDetails));
		oConnection.WaitForSendFinish(0);

		//senden des Objektes
		std::cout << iLength*sizeof(float) << std::endl;
		oConnection.Send(oMusik[0].data(), iLength * sizeof(float));
		oConnection.WaitForSendFinish(0);

		

		//oConnection.Send(&oFrame, sizeof(oFrame));
		//std::cout << sizeof(oFrame) << std::endl;
		

		std::cout << "Result channel connection successfully established" << std::endl;

		oConnection.Close( false );
	}

	pSocket->CloseSocket();

	return 0;
}
