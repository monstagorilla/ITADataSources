#include <ITADataSourceUtils.h>
#include <ITAPortaudioInterface.h>
#include <ITAStreamProbe.h>
#include <ITAFileDataSource.h>
#include <ITAException.h>
#include <ITAStringUtils.h>

#include <iostream>
#include <string>

using namespace std;

int main( int argc, char *argv[] )
{
	if( argc != 8 )
		ITA_EXCEPT1( INVALID_PARAMETER, "Syntax error: ReciprocalDAADLatencyMeasurement.exe DeviceIndexA DeviceIndexB excitation.wav recordingA.wav recordingB.wav samplerate blocklength" );
	
	try
	{
		// Arguments
		int iDeviceA = StringToInt( string( argv[ 1 ] ) );
		int iDeviceB = StringToInt( string( argv[ 2 ] ) );
		string sExcitationSignalPath = string( argv[ 3 ] );
		string sRecordingSignalPathA = string( argv[ 4 ] );
		string sRecordingSignalPathB = string( argv[ 5 ] );
		double dSampleRate = StringToDouble( string( argv[ 6 ] ) );
		int iBlockLength = StringToInt( string( argv[ 7 ] ) );


		// --- Device A ---
		
		ITAPortaudioInterface oPADeviceA( dSampleRate, iBlockLength );
		oPADeviceA.Initialize( iDeviceA );
		string sInDevice = oPADeviceA.GetDeviceName( iDeviceA );
		cout << "Input device identifier: " << sInDevice << endl;
		int iNumInputChannelsA, iNumOutputChannelsA;
		oPADeviceA.GetNumChannels( iDeviceA, iNumInputChannelsA, iNumOutputChannelsA );
		assert( iNumInputChannelsA > 1 && iNumOutputChannelsA > 1 );

		ITAFileDatasource oPlaybackA( sExcitationSignalPath, iBlockLength );
		oPADeviceA.SetPlaybackEnabled( true );
		oPADeviceA.SetPlaybackDatasource( &oPlaybackA );

		oPADeviceA.SetRecordEnabled( true );
		ITAStreamProbe oRecordStreamProbeA( oPADeviceA.GetRecordDatasource(), sRecordingSignalPathA );
		

		// --- Device B ---
		
		ITAPortaudioInterface oPADeviceB( dSampleRate, iBlockLength );
		oPADeviceB.Initialize( iDeviceB );
		sInDevice = oPADeviceB.GetDeviceName( iDeviceB );
		cout << "Input device identifier: " << sInDevice << endl;
		int iNumInputChannelsB, iNumOutputChannelsB;
		oPADeviceB.GetNumChannels( iDeviceB, iNumInputChannelsB, iNumOutputChannelsB );

		ITAFileDatasource oPlaybackB( sExcitationSignalPath, iBlockLength );
		oPADeviceB.SetPlaybackEnabled( true );
		oPADeviceB.SetPlaybackDatasource( &oPlaybackB );

		oPADeviceB.SetRecordEnabled( true );
		ITAStreamProbe oRecordStreamProbeB( oPADeviceB.GetRecordDatasource(), sRecordingSignalPathB );


		// --- Run latency measurement ---

		// Open
		oPADeviceA.Open();
		oPADeviceA.Open();

		// Start streaming
		oPADeviceA.Start();
		oPADeviceB.Start();

		// Measurement
		WriteFromDatasourceToFile( &oRecordStreamProbeA, sRecordingSignalPathA, oPlaybackA.GetFileCapacity(), 1.0, true, true );
		WriteFromDatasourceToFile( &oRecordStreamProbeB, sRecordingSignalPathA, oPlaybackB.GetFileCapacity(), 1.0, true, true );

		// Stop streaming
		oPADeviceA.Stop();
		oPADeviceB.Stop();

		// Close
		oPADeviceA.Close();
		oPADeviceB.Close();

		// Finalize
		oPADeviceA.Finalize();
		oPADeviceB.Finalize();
	}
	catch( const ITAException& e )
	{
		cerr << e << endl;
	}

	return 0;
}