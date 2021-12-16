
#include <ITAException.h>
#include <ITAFileDatasource.h>
#include <ITAStreamFilter.h>
#include <ITAStreamYJunction.h>
#include <ITAStringUtils.h>
#include <iostream>

using namespace std;

int main( int argc, char* argv[] )
{
	if( argc != 3 )
	{
		cerr << "Syntax: YJunctionTest INPUTFILE OUTPUFILE\n" << endl;
		return 255;
	}

	ITAFileDatasource* pSource = 0;
	ITAStreamProbe* pProbe     = 0;

	try
	{
		cout << "Input file: \"" << argv[1] << "\"\n" << endl;
		pSource = new ITAFileDatasource( argv[1], 1024 );
		pProbe  = new ITAStreamProbe( pSource, argv[2] );

		unsigned int n = pSource->GetCapacity( ) / pSource->GetBlocklength( );
		unsigned int c = pSource->GetNumberOfChannels( );
		for( unsigned int i = 0; i < n; i++ )
		{
			for( unsigned int j = 0; j < c; j++ )
				pProbe->GetBlockPointer( j );
			pProbe->IncrementBlockPointer( );
		}

		delete pProbe;
		delete pSource;
	}
	catch( ITAException& e )
	{
		delete pProbe;
		delete pSource;
		cerr << "Error: " << e.toString( ) << endl;
	}

	return 0;
}