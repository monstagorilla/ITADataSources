/*
   +------------------------------------------------------------+
   |                                                            |
   |  Stability04.cpp                                           |
   |                                                            |
   |  Testprogramm aus der Testsuite des ITAsioInterface        |
   |                                                            |
   |  Beschreibung:     Stablitätstest 04                       |
   |                                                            |
   |  Author:           Frank Wefers                            |
   |  Letzte Änderung:  13.04.2011 [stienen]                    |
   |                                                            |
   |  (c) Copyright Institut für technische Akustik (ITA)       |
   |      Aachen university of technology (RWTH), 2005          |
   |                                                            |
   +------------------------------------------------------------+
*/
// $Id: Stability04.cpp 1801 2011-04-13 14:07:25Z stienen $

#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAsioInterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

/*
   Testbeschreibung: 1) Während laufendem Streaming die Wiedergabedatenquelle ändern
                     2)     "       "         "     die Ausgabeverstärkung ändern
*/

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		fprintf( stderr, "Syntax: Stability04 TREIBERNUMMER\n" );

		return 255;
	}

	ITAsioInitializeLibrary( );

	long lDriver = atoi( argv[1] );

	// Zufallsgenerator initialisieren
	srand( (unsigned)time( NULL ) );

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

		ITAFileDatasource source1( "in1.wav", (unsigned int)lBuffersize, true );
		ITAFileDatasource source2( "in2.wav", (unsigned int)lBuffersize, true );

		if( ITAsioCreateBuffers( 0, 2, lBuffersize ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioCreateBuffers schlug fehl!\n" );

			return 255;
		}

		if( ITAsioStart( ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioStart schlug fehl!\n" );

			return 255;
		}

		printf( "Wiedergabe gestartet ...\n" );

		int n = 12;
		for( int i = 0; i < n; i++ )
		{
			printf( "Durchgang #%d von #%d\n\n", i + 1, n );

			Sleep( (int)( ( (double)rand( ) ) / ( (double)RAND_MAX ) * 5000 ) );

			printf( "Wechsele Datenquelle ... " );
			switch( rand( ) % 3 )
			{
				case 0:
					printf( "Keine Quelle\n" );
					ITAsioSetPlaybackDatasource( 0 );
					break;

				case 1:
					printf( "Quelle 1\n" );
					ITAsioSetPlaybackDatasource( &source1 );
					break;

				case 2:
					printf( "Quelle 2\n" );
					ITAsioSetPlaybackDatasource( &source2 );
					break;
			}
		}

		if( ITAsioStop( ) != ASE_OK )
		{
			ITAsioFinalizeLibrary( );
			fprintf( stderr, "Fehler: ITAsioStop schlug fehl!\n" );

			return 255;
		}

		printf( "Wiedergabe beendet!\n" );

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

	return 0;
}