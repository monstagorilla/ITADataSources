#include <ITAAsioInterface.h>

#include <process.h>
#include <stdio.h>
#include <string>

<<<<<<< HEAD
#ifndef  _WIN32_WINNT // @todo: remove
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

int test( const char* pszDriverName )
{
=======
#include <windows.h>

int test( const char* pszDriverName ) {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
	ITAsioInitializeLibrary();

	// Treiber anzeigen
	long lDrivers = ITAsioGetNumDrivers();
<<<<<<< HEAD
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
=======
	if( lDrivers > 0 ) {
		printf( "Folgende ASIO-Treiber wurden gefunden:\n\n" );
		for( long i = 0; i < lDrivers; i++ ) {
			printf( "\t[%d]\t%s\n", i + 1, ITAsioGetDriverName( i ) );
		}
	}
	else {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
		printf( "Keine ASIO-Treiber gefunden!\n" );
		return 0;
	}

	// Treiber initialisieren
	printf( "Initialisiere Treiber \"%s\"\n", pszDriverName );
	ASIOError ae = ITAsioInitializeDriver( pszDriverName );
<<<<<<< HEAD
	if( ae == ASE_OK )
	{
		printf( "Initialisierung erfolgreich!" );
		ITAsioFinalizeDriver();
	}
	else
	{
		printf( "Initialisierung fehlgeschlagen (errorcode %d: %s)\n", ae, ITAsioGetErrorStr( ae ) );
=======
	if( ae == ASE_OK ) {
		printf( "Initialisierung erfolgreich!" );
		ITAsioFinalizeDriver();
	}
	else {
		printf( "Initialisierung fehlgeschlagen (errorcode %d: %s)\n",
			ae, ITAsioGetErrorStr( ae ) );
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
		return -1;
	}

	ITAsioFinalizeLibrary();

	return 0;
}

<<<<<<< HEAD
void ThreadProc( void* pParam )
{
=======
void ThreadProc( void* pParam ) {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
	printf( "Thread ist gestartet!\n" );

	// COM initialisieren (Appartment)
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

<<<<<<< HEAD
	if( test( ( const char* ) pParam ) != 0 )
	{
=======
	if( test( ( const char* ) pParam ) != 0 ) {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
		//CoUninitialize();
	}

	_endthread();
}

<<<<<<< HEAD
int main( int argc, char* argv[] )
{
=======
int main( int argc, char* argv[] ) {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576

	// COM initialisieren (Appartment)
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

<<<<<<< HEAD
	if( argc != 2 )
	{
=======
	if( argc != 2 ) {
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576
		fprintf( stderr, "Syntax: ThreadInit TREIBERNUMMER\n" );
		return 255;
	}

	bool bThreaded = true;

	if( bThreaded ) {
		HANDLE hThread = ( HANDLE ) _beginthread( ThreadProc, 0, argv[ 1 ] );
		WaitForSingleObject( hThread, INFINITE );
	}
	else
<<<<<<< HEAD
	{
		test( argv[ 1 ] );
	}
=======
		test( argv[ 1 ] );
>>>>>>> 87b82f9a45babe4a44bc6ff4bdb51601b26b3576

	//CoUninitialize();

	return 0;
}