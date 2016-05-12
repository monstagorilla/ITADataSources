#include <ITAException.h>
#include <ITAAsioInterface.h>
#include <ITAFileDatasource.h>
#include <ITAFilesystemUtils.h>
#include <ITAStreamPatchbay.h>

#include <stdio.h>
#include <windows.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Syntax: play TREIBERNUMMER EINGABEDATEI\n");

		return 255;
	}

	ITAsioInitializeLibrary();

	long lDriver = atoi(argv[1]);
	
	try {

		printf("Benutze Treiber [%i]: \"%s\"\n", lDriver, ITAsioGetDriverName(lDriver-1));

		if (ITAsioInitializeDriver(lDriver-1) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioInit schlug fehl!\n");

			return 255;
		}		

		long lBuffersize, lDummy;
		if (ITAsioGetBufferSize(&lDummy, &lDummy, &lBuffersize, &lDummy) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioGetBufferSize schlug fehl!\n");

			return 255;
		}

		std::string sFilePath = correctPath( argv[2] );
		ITAFileDatasource source( sFilePath, (signed int) lBuffersize, false);

		if (ITAsioSetSampleRate((ASIOSampleRate) source.GetSamplerate()) != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioSetSamplerate schlug fehl!\n");

			return 255;
		}

		long lNumInputChannels, lNumOutputChannels;
		ASIOError ae;
		if( ( ae = ITAsioGetChannels( &lNumInputChannels, &lNumOutputChannels ) ) != ASE_OK )
		{
			cerr << "Error in ITAsioGetChannels, ASIO error " << ae << " encountered" << endl;
			ITAsioFinalizeLibrary();
			return 255;
		}

		if( ( ae = ITAsioCreateBuffers( lNumInputChannels, lNumOutputChannels, lBuffersize ) ) != ASE_OK )
		{
			cerr << "Error in ITAsioCreateBuffers, ASIO error " << ae << " encountered" << endl;
			ITAsioFinalizeLibrary();
			return 255;
		}

		ITAStreamPatchbay patchbay( source.GetSamplerate(), lBuffersize );
		patchbay.AddInput( &source );
		patchbay.AddOutput( int( lNumOutputChannels ) );
		
		for( int n=0; n<lNumOutputChannels; n++ )
		{
			if( source.GetNumberOfChannels() > 1 )
				patchbay.ConnectChannels( 0, n % ( source.GetNumberOfChannels() - 1 ), 0, n );
			else
				patchbay.ConnectChannels( 0, 0, 0, n );
		}

		ITAsioSetPlaybackDatasource( patchbay.GetOutputDatasource( 0 ) );

		if (ITAsioStart() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStart schlug fehl!\n");

			return 255;
		}

		printf("Wiedergabe gestartet ...\n");
		while (source.GetCursor() < source.GetCapacity()) Sleep(100);
		
		if (ITAsioStop() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioStop schlug fehl!\n");

			return 255;
		}

		printf("Wiedergabe beendet!\n");

		if (ITAsioDisposeBuffers() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioDisposeBuffers schlug fehl!\n");

			return 255;
		}

		if (ITAsioFinalizeDriver() != ASE_OK) {
			ITAsioFinalizeLibrary();
			fprintf(stderr, "Fehler: ITAsioExit schlug fehl!\n");

			return 255;
		}
	} catch (ITAException e) {
		ITAsioFinalizeLibrary();
		cerr << e << endl;

		return 255;
	}
	
	ITAsioFinalizeLibrary();

	return 0;
}