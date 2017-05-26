#include <ITAAsioInterface.h>

#include <process.h>
#include <stdio.h>
#include <string>

#ifndef  _WIN32_WINNT // @todo: remove
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>


int test( const char* pszDriverName )
{
	ITAsioInitializeLibrary();

	// Treiber anzeigen
	long lDrivers = ITAsioGetNumDrivers();
	if( lDrivers > 0 )
	{
		printf( "Folgende ASIO-Treiber wurden gefunden:\n\n" );
		for( long i = 0; i < lDrivers; i++ )
		{
			printf( "\t[%d]\t%s\n", i + 1, ITAsioGetDriverName( i ) );
		}
	}
	else
	{
		printf( "Keine ASIO-Treiber gefunden!\n" );
		return 0;
	}

	// Treiber initialisieren
	printf( "Initialisiere Treiber \"%s\"\n", pszDriverName );
	ASIOError ae = ITAsioInitializeDriver( pszDriverName );
	if( ae == ASE_OK )
	{
		printf( "Initialisierung erfolgreich!" );
		ITAsioFinalizeDriver();
	}
	else
	{
		printf( "Initialisierung fehlgeschlagen (errorcode %d: %s)\n", ae, ITAsioGetErrorStr( ae ) );

		return -1;
	}

	ITAsioFinalizeLibrary();

	return 0;
}

void ThreadProc( void* pParam )
{
	printf( "Thread ist gestartet!\n" );

	// COM initialisieren (Appartment)
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if( test( ( const char* ) pParam ) != 0 )
	{
		//CoUninitialize();
	}

	_endthread();
}

int main( int argc, char* argv[] )
{

	// COM initialisieren (Appartment)
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if( argc != 2 )
	{
		fprintf( stderr, "Syntax: ThreadInit TREIBERNUMMER\n" );
		return 255;
	}

	bool bThreaded = true;

	if( bThreaded )
	{
		HANDLE hThread = ( HANDLE ) _beginthread( ThreadProc, 0, argv[ 1 ] );
		WaitForSingleObject( hThread, INFINITE );
	}
	else
	{
		test( argv[ 1 ] );
	}

	//CoUninitialize();

	return 0;
}