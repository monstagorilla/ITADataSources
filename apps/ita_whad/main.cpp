//#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <string>

#include <ITAAsioInterface.h>

using namespace std;

int main( int argc, char* argv[] )
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

	string sFileName = "ita_whad_asio.txt";
	FILE* file = fopen( sFileName.c_str(), "w" );

	ITAsioInitializeLibrary();

	long lDrivers = ITAsioGetNumDrivers();
	
	if( lDrivers == 0 )
		cerr << "Warning: no ASIO drivers found." << endl;

	for( long i = 0; i < lDrivers; i++ )
	{
		cout << "[" << i+1 << "] \"" << ITAsioGetDriverName(i) << "\"" << endl;
	}

	ITAsioFinalizeLibrary();

	fclose( file );

#endif // ITA_WHAD_WITH_ASIO

	return 0;
}
