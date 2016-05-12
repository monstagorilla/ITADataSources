
#include <ITAStreamPatchbay.h>
#include <ITAFileDatasource.h>
#include <ITAFileDatasink.h>
#include <ITADatasource.h>
#include <ITADatasourceUtils.h>
#include <ITAException.h>
#include <iostream>

using namespace std;

const static double dSampleRate = 44.1e3;
const static int iBlockLength = 128;
const static std::string sFileName = "../../../VAData/Audiofiles/Bauer.wav";

void test_jst()
{
	ITAFileDatasource fs1( sFileName, iBlockLength, false );
	ITAFileDatasource fs2( sFileName, iBlockLength, false );
	ITAFileDatasource fs3( sFileName, iBlockLength, false );

	unsigned int uiSamples = fs1.GetCapacity();

	ITAStreamPatchbay pb( dSampleRate, iBlockLength );
	pb.AddInput( &fs1 );
	pb.AddInput( &fs2 );
	pb.AddInput( &fs3 );
	pb.AddInput( &fs2 );
	pb.AddInput( &fs3 );
	pb.AddInput( &fs2 );

	ITADatasource* pOut = pb.GetOutputDatasource( pb.AddOutput( 6 ) );

	int iOutChannel=0;
	pb.ConnectChannels( 0, 0, 0, iOutChannel++, 1.0f ); // OK
	pb.ConnectChannels( 1, 0, 0, iOutChannel++, 0.8f ); // fs2
	pb.ConnectChannels( 2, 0, 0, iOutChannel++, 0.6f ); // fs3
	pb.ConnectChannels( 1, 0, 0, iOutChannel++, 0.3f ); // fs2
	pb.ConnectChannels( 2, 0, 0, iOutChannel++, 0.2f ); // fs3 -> 2x block inc
	pb.ConnectChannels( 1, 0, 0, iOutChannel++, 0.5f ); // fs2 -> 3x block inc

	pb.PrintConnections();

	ITAFileDatasink sink1( "out_jst.wav", pOut );
	sink1.Transfer( uiSamples );

	pb.DisconnectAllOutputs();
}

void test1() {
	const unsigned int uiBlocklength = 1024;

	/*ITAFileDatasource* fs1 = new ITAFileDatasource("ones.wav", uiBlocklength, false);*/
	ITAFileDatasource* fs2 = new ITAFileDatasource("Bauer.wav", uiBlocklength, false);
	//ITAFileDatasource* fs3 = new ITAFileDatasource("bla.wav", uiBlocklength, false);

	ITAStreamPatchbay* pb = new ITAStreamPatchbay(44100,uiBlocklength);
	
/*	pb->AddInput(1);*/
	pb->AddInput(1);
	pb->AddOutput(8);


	pb->SetInputDatasource(0, fs2);
	/*pb->SetInputDatasource(1, fs2);*/

	/*
	pb->ConnectChannels(1,1,0,2);
	pb->ConnectChannels(0,0,0,1);
	pb->ConnectChannels(0,0,0,0);
	pb->ConnectChannels(1,0,0,3);
	*/
	pb->ConnectChannels(0,0,0,0,0.2);
	pb->ConnectChannels(0,0,0,1,1.0);
	pb->ConnectChannels(0,0,0,2,0.0);
	pb->ConnectChannels(0,0,0,3,1.0);
	pb->ConnectChannels(0,0,0,4,0.0);
	pb->ConnectChannels(0,0,0,5,0.0);
	pb->ConnectChannels(0,0,0,6,1.0);
	pb->ConnectChannels(0,0,0,7,0.0);
	
	
	ITAFileDatasink sink1("out.wav",pb->GetOutputDatasource(0));

	pb->SetInputGain(0,1);
	
	
	sink1.Transfer(100000);
	
	/*pb->SetOutputGain(0,0.1);*/


	sink1.Transfer(1000);
	
	pb->SetInputMuted(0,true);
	sink1.Transfer(1000);

	pb->SetInputMuted(0,false);
	sink1.Transfer(1000);
	/*pb->SetOutputGain(0,.8);*/
	sink1.Transfer(1000);
	
	pb->PrintConnections();

	/*WriteFromDatasourceToFile(pb->GetOutputDatasource(0), "out.wav", 44100*10, 1.0, false, true);*/
	
	/*delete fs1;*/
	delete fs2;
	delete pb;
}


int main() 
{
	try
	{
		//test1();
		test_jst();
	}
	catch( const ITAException& e )
	{
		cerr << e << endl;
		return 255;
	}
	return 0;
}