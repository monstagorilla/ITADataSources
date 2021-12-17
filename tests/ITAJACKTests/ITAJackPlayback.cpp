
#include <ITADataSourceUtils.h>
#include <ITAException.h>
#include <ITAFileDataSource.h>
#include <ITAJACKInterface.h>
#include <ITAStreamFunctionGenerator.h>
#include <iostream>
#include <string>


void playback( std::string fileName )
{
	ITAJACKInterface jack;

	auto err = jack.Initialize( "jack-playback" );
	if( err != ITAJACKInterface::ITA_JACK_NO_ERROR )
	{
		std::cerr << "Failed to init jack!" << std::endl;
		return;
	}

	jack.printInfo( );

	int blockSize = jack.GetBlockSize( );

	ITADatasource* pSource = NULL;

	try
	{
		pSource = new ITAFileDatasource( fileName, blockSize, true );
	}
	catch( ITAException& e )
	{
		std::cerr << "Could open audio file, error = " << e << ". Falling back to sine signal" << std::endl;
		pSource = new ITAStreamFunctionGenerator( jack.GetNumOutputChannels( ), jack.GetSampleRate( ), blockSize, ITAStreamFunctionGenerator::SINE, 300, 0.9f, true );
	}

	jack.SetPlaybackDatasource( pSource );

	jack.Open( );
	jack.Start( );

	std::cout << "Started. Press any key to quit." << std::endl;
	getchar( );

	jack.Stop( );

	jack.Close( );
	jack.Finalize( );

	return;
}

int main( int argc, char* argv[] )
{
	std::string fileName( argc > 1 ? argv[1] : "sine" );
	std::cout << "Starting playback of " << fileName << std::endl;
	playback( fileName );

	return 0;
}