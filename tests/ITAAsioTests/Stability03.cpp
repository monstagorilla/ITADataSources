/*
   +------------------------------------------------------------+
   |                                                            |
   |  Stability03.cpp                                           |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Stablitätstest 03                       |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  13.04.2011 [stienen]                    |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |
   +------------------------------------------------------------+
*/
// $Id: Stability03.cpp 1801 2011-04-13 14:07:25Z stienen $

#include <ITAException.h>
#include <ITAsioInterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

/*
   Testbeschreibung: 1) ASIO-Streaming mehrmals direkt hintereinander
                        starten (ITAsioStart) und stoppen (ITAsioStop).

                     2) ASIO-Streaming unterschiedlich lang laufen lassen (zufällig)
*/

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		fprintf( stderr, "Syntax: Stability03 TREIBERNUMMER\n" );
		return 255;
	}

	ITAsioInitializeLibrary( );

	// Zufallsgenerator initialisieren
	srand( (unsigned)time( NULL ) );

	long lDriver = atoi( argv[1] );

	try
	{
		if( ITAsioInitializeDriver( lDriver - 1 ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioInit schlug fehl!\n" );

			return 255;
		}

		long lBuffersize, lDummy;
		if( ITAsioGetBufferSize( &lDummy, &lDummy, &lBuffersize, &lDummy ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioGetBufferSize schlug fehl!\n" );

			return 255;
		}

		long lInputChannels, lOutputChannels;
		if( ITAsioGetChannels( &lInputChannels, &lOutputChannels ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioGetChannels schlug fehl!\n" );

			return 255;
		}

		if( ITAsioCreateBuffers( lInputChannels, lOutputChannels, lBuffersize ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n" );

			return 255;
		}

		printf( "Teste direktes Starten und Stoppen\n\n" );
		int n = 10;
		for( int i = 0; i < n; i++ )
		{
			printf( "Durchgang #%d von #%d\n\n", i + 1, n );
			printf( "-> ITAsioStart\n" );
			if( ITAsioStart( ) != ASE_OK )
			{
				ITAsioFinalizeLibrary( );
				fprintf( stderr, "Fehler: ITAsioStart schlug fehl!\n" );

				return 255;
			}

			printf( "-> ITAsioStop\n" );
			if( ITAsioStop( ) != ASE_OK )
			{
				ITAsioFinalizeLibrary( );
				fprintf( stderr, "Fehler: ITAsioStop schlug fehl!\n" );

				return 255;
			}
		}

		printf( "\nTeste zufaelliges Starten und Stoppen\n\n" );
		for( int i = 0; i < n; i++ )
		{
			printf( "Durchgang #%d von #%d\n\n", i + 1, n );
			printf( "-> ITAsioStart\n" );
			if( ITAsioStart( ) != ASE_OK )
			{
				ITAsioFinalizeLibrary( );
				fprintf( stderr, "Fehler: ITAsioStart schlug fehl!\n" );

				return 255;
			}

			Sleep( (int)( ( (double)rand( ) ) / ( (double)RAND_MAX ) * 5000 ) );

			printf( "-> ITAsioStop\n" );
			if( ITAsioStop( ) != ASE_OK )
			{
				ITAsioFinalizeLibrary( );
				fprintf( stderr, "Fehler: ITAsioStop schlug fehl!\n" );

				return 255;
			}

			Sleep( (int)( ( (double)rand( ) ) / ( (double)RAND_MAX ) * 5 ) );
		}

		if( ITAsioDisposeBuffers( ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n" );

			return 255;
		}

		if( ITAsioFinalizeDriver( ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioExit schlug fehl!\n" );

			return 255;
		}
	}
	catch( ITAException& e )
	{
		ITAsioFinalizeLibrary( );
		fprintf( stderr, "Fehler: %s\n", e.toString( ).c_str( ) );

		return 255;
	}

	ITAsioFinalizeLibrary( );

	printf( "\nTest beendet\n" );

	return 0;
}