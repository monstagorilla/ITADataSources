#include <ITAPortaudioInterface.h>
#include <ITADataSource.h>
#include <ITADataSourceUtils.h>
#include <ITABufferDataSink.h>
#include <ITASampleFrame.h>

#include <iostream>
#include <string>
#include <vector>

static double g_dSampleRate = 44.1e3;
static int g_iBlockSize = 512;
static std::string g_sOutputFileName = "ITAPA_Record.wav";
float g_fRecordingTime = 5; // Seconds

void record()
{

	ITAPortaudioInterface ITAPA( g_dSampleRate, g_iBlockSize );

	int iInputDevice = ITAPA.GetDefaultInputDevice();
	//iInputDevice = 4;

	ITAPortaudioInterface::ITA_PA_ERRORCODE err;
	err = ITAPA.Initialize( iInputDevice );
	ITAPA.SetPlaybackEnabled( false );
	ITAPA.SetRecordEnabled( true );
	ITADatasource* pdsRecordDatasource = ITAPA.GetRecordDatasource();
	ITAPA.Open();

	unsigned int nRecordSamples = ( unsigned int ) ( 5 * g_dSampleRate );
	ITASampleFrame sfRecordData( pdsRecordDatasource->GetNumberOfChannels(), nRecordSamples, true );

	std::vector< float* > vpfRecordData( sfRecordData.channels() );
	for( int i = 0; i < sfRecordData.channels(); i++ )
		vpfRecordData[ i ] = sfRecordData[ i ].data();

	ITAPA.Start();
	//ITABufferDataSink oBufferRec( pdsRecordDatasource, vpfRecordData, g_iBlockSize );
	//oBufferRec.Transfer( nRecordSamples );
	WriteFromDatasourceToFile( pdsRecordDatasource, g_sOutputFileName, (unsigned int)(5*g_dSampleRate), 1.0, true, true );
	ITAPA.Stop();

	ITAPA.Close();
	ITAPA.Finalize();
	
	sfRecordData.Store( g_sOutputFileName, g_dSampleRate );

	return;
}

int main( int , char** )
{
	std::cout << "Starting recording ..." << std::endl;
	record();

	return 0;
}