#include <ITADataSourceUtils.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamAmplifier.h>
#include <iostream>
#include <stdio.h>

using namespace std;

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		fprintf( stderr, "Syntax: FileSourceTest INPUTFILE\n" );
		return 255;
	}

	ITADatasource* pSource = 0;

	try
	{
		printf( "Input file: \"%s\"\n", argv[1] );
		pSource = new ITAFileDatasource( argv[1], 1024 );

		printf( "Channels:       %d\n", pSource->GetNumberOfChannels( ) );
		printf( "Capacity:       %d Samples\n", ( (ITAFileDatasource*)pSource )->GetCapacity( ) );
		printf( "File capacity:  %d Samples\n", ( (ITAFileDatasource*)pSource )->GetFileCapacity( ) );
		printf( "Samplerate:     %0.3f KHz\n", pSource->GetSampleRate( ) / 1000 );

		/*		unsigned int n=1;
		        while (pSource->GetCursor() < pSource->GetCapacity()) {
		            printf("Fetching blocks #%d (Cursor = %d)\n", n++, pSource->GetCursor());
		            for (unsigned int i=0; i<pSource->GetNumberOfChannels(); i++) {
		                const float* pfData = pSource->GetBlockPointer(i);
		                // ...
		            }
		            pSource->IncrementBlockPointer();
		        }
		*/
		ITAStreamAmplifier* pAmp = new ITAStreamAmplifier( pSource );
		pAmp->SetGain( 0.1f );


		// WriteFromDatasourceToFile(pSource, "Out.wav", pSource->GetFileCapacity()+100000, 1.0, false, true);
		WriteFromDatasourceToFile( pAmp, "Out.wav", ( (ITAFileDatasource*)pSource )->GetFileCapacity( ) + 100000, 1.0, false, true );

		/*
		        pSource->SetROI(1000, 6000);
		        pSource->SetLoopMode(true);
		        pSource->Rewind();
		        WriteFromDatasourceToFile(pSource, "Loop.wav", pSource->GetROILength()*10, 1.0, false, true);
		*/
		delete pSource;
	}
	catch( ITAException& e )
	{
		delete pSource;
		cerr << e << endl;
	}

	return 0;
}
