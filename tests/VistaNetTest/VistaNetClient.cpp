#include <ITAAudiofileWriter.h>
#include <ITASampleFrame.h>
#include <ITAStopWatch.h>
#include <ITAStringUtils.h>
#include <VistaInterProcComm/Connections/VistaConnectionIP.h>
#include <VistaInterProcComm/IPNet/VistaIPAddress.h>
#include <VistaInterProcComm/IPNet/VistaTCPServer.h>
#include <VistaInterProcComm/IPNet/VistaTCPSocket.h>
#include <iostream>
#include <string>


int main( int argc, char** argv )
{
	if( argc != 3 )
	{
		std::cout << "Syntax error: VistaNetClient SERVER PORT" << std::endl;
		return 255;
	}

	std::string sServerName = argv[1];
	int iServerPort         = StringToInt( std::string( argv[2] ) );

	std::cout << "Attempting to connect to '" << sServerName << ":" << iServerPort << "'" << std::endl;

	ITAStopWatch sw;
	sw.start( );
	VistaConnectionIP oCommandChannelConnection( VistaConnectionIP::CT_TCP, sServerName, iServerPort );

	if( oCommandChannelConnection.GetIsConnected( ) == false )
	{
		std::cerr << "Connection error, exiting." << std::endl;
		return 255;
	}

	std::string sLocalAddress;
	oCommandChannelConnection.GetLocalAddress( ).GetIPAddress( ).GetAddressString( sLocalAddress );

	int iBackchannelPort = 12481;
	VistaTCPServer oServer( sLocalAddress, iBackchannelPort, 1 );

	if( oServer.GetIsValid( ) == false )
	{
		std::cerr << "Could not start server" << std::endl;
		return 255;
	}

	// Transmit port on command channel
	oCommandChannelConnection.WriteRawBuffer( &iBackchannelPort, 8 );

	VistaTCPSocket* pSocket = oServer.GetNextClient( );
	unsigned long l         = pSocket->WaitForIncomingData( 0 );
	std::cout << "l = " << l << std::endl;
	if( l == sizeof( VistaType::byte ) )
	{
		bool bAck;
		pSocket->ReceiveRaw( &bAck, l );
		std::cout << "Client received acknowledge flag '" << bAck << "'" << std::endl;
	}
	// enpfangen bool array
	bool bArr[5];
	if( l == sizeof( bArr ) )
	{
		pSocket->ReceiveRaw( &bArr, l );
		std::cout << "bool array is '";
		for( int i = 0; i < 5; i++ )
		{
			std::cout << bArr[i] << ' ';
		}
		std::cout << std::endl;
	}
	// empfangen Int-Array
	int iArr[6];
	// std::cout << "iArr = " << sizeof(iArr) << std::endl;
	if( l == sizeof( iArr ) )
	{
		pSocket->ReceiveRaw( &iArr, l );
		std::cout << "int array is '";
		for( int i = 0; i < 5; i++ )
		{
			std::cout << iArr[i] << ' ';
		}
		std::cout << std::endl;
	}

	// Empfangen der wave datei
	// empfangen der Infos zur Übertragung
	int iDetails[3];
	std::cout << "iDetails = " << sizeof( iDetails ) << std::endl;
	if( l == sizeof( iDetails ) )
	{
		pSocket->ReceiveRaw( &iDetails, l );
		std::cout << "iLength: " << iDetails[0] << "\niChannels: " << iDetails[1] << "\niFrequency: " << iDetails[2] << std::endl;
		// oCommandChannelConnection.Close(false);

		unsigned long m = pSocket->WaitForIncomingData( 0 );
		std::cout << "l = " << m << std::endl;
		ITASampleFrame oMusik( iDetails[1], iDetails[0], true );
		ITASampleFrame otest( iDetails[1], iDetails[0], true );
		// schleife für jeden channel

		// pSocket->ReceiveRaw(oMusik[0].data(), iDetails[0] * iDetails[2]);
		/*
		pSocket->ReceiveRaw(oMusik[0].data(), m);
		pData += m;
		m = pSocket->WaitForIncomingData(0);
		pSocket->ReceiveRaw(oMusik[0].data()+m, m);
		*/

		int iDiff = 0;

		while( iDiff < iDetails[0] )
		{
			m = pSocket->WaitForIncomingData( 0 );
			pSocket->ReceiveRaw( oMusik[0].data( ) + iDiff, m );
			iDiff += m / 4;
			std::cout << "Fortschritt = " << iDiff << " / " << iDetails[0] << std::endl;
		}

		writeAudiofile( "EmpfMus.wav", &oMusik, iDetails[2], ITAQuantization::ITA_INT16 );
		oCommandChannelConnection.Close( false );
	}


	return 0;
}
