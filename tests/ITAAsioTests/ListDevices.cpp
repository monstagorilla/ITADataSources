#include <ITAAsioInterface.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <windows.h>

using namespace std;

int main( int argc, char* argv[] )
{
	ITAsioInitializeLibrary( );

	long lDrivers = ITAsioGetNumDrivers( );

	if( lDrivers > 0 )
	{
		cout << "Folgende ASIO-Treiber wurden gefunden:" << endl << endl;
		for( long i = 0; i < lDrivers; i++ )
			cout << "[" << i + 1 << "] \"" << ITAsioGetDriverName( i ) << "\"" << endl;
	}
	else
		cout << "Keine ASIO-Treiber gefunden" << endl;

	ITAsioFinalizeLibrary( );

	return 0;
}
