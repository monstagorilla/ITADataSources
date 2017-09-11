#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAStreamDetector.h>
#include <ITAStringUtils.h>
#include <iostream>

using namespace std;

int main( int argc, char* argv[] )
{

	if( argc != 2 )
	{
		cerr << "Syntax: StreamDetectorText INPUTFILE\n" << endl;
		return 255;
	}

	ITAFileDatasource* pSource = 0;
	ITAStreamDetector* pDetector = 0;

	try
	{
		cout << "Input file: \"" << argv[ 1 ] << "\"\n" << endl;
		pSource = new ITAFileDatasource( argv[ 1 ], 1024 );
		pDetector = new ITAStreamDetector( pSource, ITAStreamDetector::PEAK_AND_RMS );

		int iNumBlocks = pSource->GetCapacity() / pSource->GetBlocklength();
		int iNumChannels = pSource->GetNumberOfChannels();
		for( int i = 0; i < iNumBlocks; i++ )
		{
			for( int j = 0; j < iNumChannels; j++ )
				pDetector->GetBlockPointer( j, 0 );
			pDetector->IncrementBlockPointer();
		}

		vector< double > vdPeaksDecibel;
		pDetector->GetPeaksDecibel( vdPeaksDecibel );

		for( unsigned int i = 0; i < vdPeaksDecibel.size(); i++ )
			cout << "Peak channel " << ( i + 1 ) << ": " << DecibelToString( vdPeaksDecibel[ i ] ) << endl;

		int iChannel = -1;
		double dOverallPeakdB = pDetector->GetOverallPeakDecibel( iChannel );
		cout << endl << "Max peak over all channels: " << DecibelToString( dOverallPeakdB ) << " in channel " << ( iChannel + 1 ) << endl << endl;

		vector< float > vfRMSDecibel;
		pDetector->GetRMSsDecibel( vfRMSDecibel, false ); // No reset here, otherwise we can not obtain overall RMS anymore

		for( unsigned int i = 0; i < vfRMSDecibel.size(); i++ )
			cout << "RMS channel " << ( i + 1 ) << ": " << DecibelToString( vfRMSDecibel[ i ] ) << endl;

		double dOverallRMSdB = pDetector->GetOverallRMSDecibel();
		cout << endl << "RMS over all channels: " << DecibelToString( dOverallRMSdB ) << endl;


		delete pDetector;
		delete pSource;
	}
	catch( ITAException& e )
	{
		delete pDetector;
		delete pSource;
		cerr << "Error: " << e << endl;
	}

	return 0;
}
