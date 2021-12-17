#include <iostream>
#include <stdio.h>
#include <string>

#ifdef ITA_WHAD_WITH_ASIO
#	include <ITAAsioInterface.h>
#endif

#ifdef ITA_WHAD_WITH_PORTAUDIO
#	include <ITAPortaudioInterface.h>
#endif

using namespace std;

int main( int, char** )
{
	cout << "ita_whad - ITA's which audio devices application" << endl;
	cout << " ... lists all available audio devices and available properties." << endl << endl;
#ifdef ITA_WHAD_WITH_ASIO
	cout << "ASIO support: yes" << endl;
#else
	cout << "ASIO support: no" << endl;
#endif // ITA_WHAD_WITH_ASIO
#ifdef ITA_WHAD_WITH_PORTAUDIO
	cout << "Portaudio support: yes" << endl;
#else
	cout << "Portaudio support: no" << endl;
#endif // ITA_WHAD_WITH_PORTAUDIO
	cout << endl;


#ifdef ITA_WHAD_WITH_ASIO

	string sASIOFileName = "ita_whad_asio.txt";
	FILE* pASIOFile      = fopen( sASIOFileName.c_str( ), "w" );

	ITAsioInitializeLibrary( );

	long lASIODrivers = ITAsioGetNumDrivers( );

	if( lASIODrivers == 0 )
		cerr << "Warning: no ASIO drivers found." << endl;

	cout << " ### ASIO ### " << endl;
	for( long i = 0; i < lASIODrivers; i++ )
	{
		std::string sDriverName = ITAsioGetDriverName( i );
		ASIOError ae            = ITAsioInitializeDriver( i );
		long iIn = -1, iOut = -1;

		if( ae == ASE_OK )
		{
			ITAsioGetChannels( &iIn, &iOut );
			ITAsioFinalizeDriver( );
		}

		cout << "[" << i + 1 << "] \"" << sDriverName << "\" (" << iIn << " in, " << iOut << " out)" << endl;
	}
	cout << endl;

	ITAsioFinalizeLibrary( );

	fclose( pASIOFile );

#endif // ITA_WHAD_WITH_ASIO
#ifdef ITA_WHAD_WITH_PORTAUDIO

	string sPAFileName = "ita_whad_portaudio.txt";
	FILE* pPAFile      = fopen( sPAFileName.c_str( ), "w" );

	ITAPortaudioInterface oITAPA( 44.1e3, 1024 );
	oITAPA.Initialize( );
	int iPANumDevices = oITAPA.GetNumDevices( );
	int iPADefaultIn  = oITAPA.GetDefaultInputDevice( );
	int iPADefaultOut = oITAPA.GetDefaultOutputDevice( );

	cout << " ### Portaudio ### " << endl;
	for( int i = 0; i < iPANumDevices; i++ )
	{
		string sExtra = "";
		if( i == iPADefaultIn || i == iPADefaultOut )
			sExtra = " *";

		cout << "[" << i + 1 << "] \"" << oITAPA.GetDeviceName( i ) << "\"" << sExtra << endl;
	}
	cout << endl;

	oITAPA.Finalize( );

	fclose( pPAFile );

#endif // ITA_WHAD_WITH_PORTAUDIO

	return 0;
}
