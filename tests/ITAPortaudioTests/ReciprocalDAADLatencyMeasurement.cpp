// ITADataSources
#include <ITADataSourceUtils.h>
#include <ITAFileDataSource.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamMultiplier1N.h>
#include <ITAStreamProbe.h>

// ITABase
#include <ITAException.h>
#include <ITAStringUtils.h>

// STL
#include <iostream>
#include <string>

using namespace std;
void list_portaudio_devices( );

int main( int argc, char* argv[] )
{
	if( argc != 8 )
		ITA_EXCEPT1( INVALID_PARAMETER,
		             "Syntax error: ReciprocalDAADLatencyMeasurement.exe DeviceIndexA DeviceIndexB excitation.wav recordingA.wav recordingB.wav samplerate blocklength" );

	try
	{
		// Arguments
		const int iDeviceA           = StringToInt( string( argv[1] ) ) - 1;
		const int iDeviceB           = StringToInt( string( argv[2] ) ) - 1;
		string sExcitationSignalPath = string( argv[3] );
		string sRecordingSignalPathA = string( argv[4] );
		string sRecordingSignalPathB = string( argv[5] );
		const double dSampleRate     = StringToDouble( string( argv[6] ) );
		const int iBlockLength       = StringToInt( string( argv[7] ) );


		// --- Device A ---

		ITAPortaudioInterface oPADeviceA( dSampleRate, iBlockLength );
		ITAPortaudioInterface::ITA_PA_ERRORCODE err;
		if( ( err = oPADeviceA.Initialize( iDeviceA ) ) != ITAPortaudioInterface::ITA_PA_NO_ERROR )
			ITA_EXCEPT1( INVALID_PARAMETER, "Could not initialize device A: " + ITAPortaudioInterface::GetErrorCodeString( err ) );

		string sDeviceA = oPADeviceA.GetDeviceName( iDeviceA );
		cout << "Device A device identifier: " << sDeviceA << endl;
		int iNumInputChannelsA, iNumOutputChannelsA;
		oPADeviceA.GetNumChannels( iDeviceA, iNumInputChannelsA, iNumOutputChannelsA );

		if( iNumInputChannelsA == 0 || iNumOutputChannelsA == 0 )
			ITA_EXCEPT1( INVALID_PARAMETER, "Not enough I/O channels for device A: " + sDeviceA );

		cout << "Using " << iNumOutputChannelsA << " outputs and " << iNumInputChannelsA << " inputs for device A" << endl;


		ITAFileDatasource oPlaybackA( sExcitationSignalPath, iBlockLength );
		if( oPlaybackA.GetNumberOfChannels( ) != 1 )
			ITA_EXCEPT_INVALID_PARAMETER( "Excitation signal must be single channel" );


		ITAStreamMultiplier1N oMultiplierA( &oPlaybackA, iNumOutputChannelsA );

		oPADeviceA.SetPlaybackEnabled( true );
		oPADeviceA.SetPlaybackDatasource( &oMultiplierA );

		oPADeviceA.SetRecordEnabled( true );
		ITAStreamProbe oRecordStreamProbeA( oPADeviceA.GetRecordDatasource( ), sRecordingSignalPathA );


		// --- Device B ---

		ITAPortaudioInterface oPADeviceB( dSampleRate, iBlockLength );
		oPADeviceB.Initialize( iDeviceB );
		string sDeviceB = oPADeviceB.GetDeviceName( iDeviceB );
		cout << "Device B device identifier: " << sDeviceB << endl;
		int iNumInputChannelsB, iNumOutputChannelsB;
		oPADeviceB.GetNumChannels( iDeviceB, iNumInputChannelsB, iNumOutputChannelsB );

		if( iNumInputChannelsB == 0 || iNumOutputChannelsB == 0 )
			ITA_EXCEPT1( INVALID_PARAMETER, "Not enough I/O channels for device B: " + sDeviceB );

		cout << "Using " << iNumOutputChannelsB << " outputs and " << iNumInputChannelsB << " inputs for device B" << endl;

		ITAFileDatasource oPlaybackB( sExcitationSignalPath, iBlockLength );
		if( oPlaybackB.GetNumberOfChannels( ) != 1 )
			ITA_EXCEPT_INVALID_PARAMETER( "Excitation signal must be single channel" );

		ITAStreamMultiplier1N oMultiplierB( &oPlaybackB, iNumOutputChannelsB );

		oPADeviceB.SetPlaybackEnabled( true );
		oPADeviceB.SetPlaybackDatasource( &oMultiplierB );

		oPADeviceB.SetRecordEnabled( true );
		ITAStreamProbe oRecordStreamProbeB( oPADeviceB.GetRecordDatasource( ), sRecordingSignalPathB );


		// --- Run latency measurement ---

		// Open
		oPADeviceA.Open( );
		oPADeviceB.Open( );

		// Start streaming
		oPADeviceA.Start( );
		oPADeviceB.Start( );

		// Measurement
		std::vector<float*> vpfSamples;
		std::vector<std::vector<float>*> vpfSamplesVector;
		for( unsigned int i = 0; i < oRecordStreamProbeB.GetNumberOfChannels( ); i++ )
		{
			std::vector<float>* pvfData = new std::vector<float>( oPlaybackA.GetFileCapacity( ) );
			vpfSamplesVector.push_back( pvfData );
			vpfSamples.push_back( &( *pvfData )[0] );
		}
		// WriteFromDatasourceToBuffer( &oRecordStreamProbeA, &vpfSamples[ 0 ], oPlaybackA.GetFileCapacity(), 1.0, true, true );
		WriteFromDatasourceToBuffer( &oRecordStreamProbeB, &vpfSamples[0], oPlaybackB.GetFileCapacity( ), 1.0, true, true );

		for( size_t i = 0; i < vpfSamplesVector.size( ); i++ )
			delete vpfSamplesVector[i];

		// Stop streaming
		oPADeviceA.Stop( );
		oPADeviceB.Stop( );

		// Close
		oPADeviceA.Close( );
		oPADeviceB.Close( );

		// Finalize
		oPADeviceA.Finalize( );
		oPADeviceB.Finalize( );
	}
	catch( const ITAException& e )
	{
		cerr << e << endl;
		cout << "Listing all available Portaudio devices: " << endl;
		list_portaudio_devices( );
	}

	return 0;
}

void list_portaudio_devices( )
{
	ITAPortaudioInterface oITAPA( 44.1e3, 1024 );
	oITAPA.Initialize( );
	int iPANumDevices = oITAPA.GetNumDevices( );
	for( int i = 0; i < iPANumDevices; i++ )
		cout << "[" << i + 1 << "] \"" << oITAPA.GetDeviceName( i ) << "\"" << endl;
	cout << endl;
	oITAPA.Finalize( );
}
