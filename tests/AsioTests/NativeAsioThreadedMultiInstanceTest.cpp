#include <cassert>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <host/asiodrivers.h>
#include <common/asio.h>

#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/Concurrency/VistaThread.h>

using namespace std;

ASIOTime* AsioBufferSwitchTimeInfo( ASIOTime *timeInfo, long index, ASIOBool processNow )
{
	/*
	for( int i = 0; i < oAsioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++ )
	{
	if( asioDriverInfo.bufferInfos[ i ].isInput == TRUE )
	{
	assert( asioDriverInfo.channelInfos[ i ].type == ASIOSTInt24LSB );
	}
	}
	*/

	return timeInfo;
};

void AsioBufferSwitch( long index, ASIOBool processNow )
{
	ASIOTime timeInfo;
	memset( &timeInfo, 0, sizeof( timeInfo ) );

	if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	AsioBufferSwitchTimeInfo( &timeInfo, index, processNow );
};

void AsioSampleRateChanged( ASIOSampleRate fs )
{
	cout << "Sample rate changed to " << fs << endl;
	return;
};

long AsioMessages( long selector, long value, void*, double* )
{
	return 0;
};

int main( int argc, char* argv[] )
{
	ASIOError ae;
	AsioDriverList* pDrivers = new AsioDriverList();
	long nNumDrivers = pDrivers->asioGetNumDev();
	cout << "Number of ASIO drivers: " << nNumDrivers << endl;
	delete pDrivers;

	for( int i = 0; i < nNumDrivers; i++ )
	{
		AsioDrivers* pDriver = new AsioDrivers();

		string sDriverName;
		sDriverName.resize( 256 );
		pDriver->asioGetDriverName( i, &sDriverName[ 0 ], sDriverName.size() );
		sDriverName = std::string( sDriverName.c_str() ); // Strip
		cout << "Driver name: " << sDriverName << "." << endl;

		bool bLoadSuccess = pDriver->loadDriver( &sDriverName[ 0 ] );
		if( bLoadSuccess )
		{
			string sDriverPath;
			sDriverPath.resize( 256 );
			pDriver->asioGetDriverPath( i, &sDriverPath[ 0 ], sDriverPath.size() );
			sDriverPath = std::string( sDriverPath.c_str() ); // Strip
			cout << "Driver path: " << sDriverPath << endl;

			cout << "Current driver index: " << pDriver->getCurrentDriverIndex() << endl;

			string sCurrentDriverName;
			sCurrentDriverName.resize( 256 );
			pDriver->getCurrentDriverName( &sCurrentDriverName[ 0 ] );
			sCurrentDriverName = std::string( sCurrentDriverName.c_str() ); // Strip
			cout << "Current driver name: " << sCurrentDriverName << endl;


			ASIODriverInfo oDriverInfo;
			oDriverInfo.asioVersion = 2;
			if( ASIOInit( &oDriverInfo ) != ASE_OK )
			{
				cout << "Could not initialize " << sCurrentDriverName << ", skipping." << endl;
				continue;
			}

			ASIOCallbacks oAsioCallback;
			oAsioCallback.asioMessage = &AsioMessages;
			oAsioCallback.bufferSwitch = &AsioBufferSwitch;
			oAsioCallback.bufferSwitchTimeInfo = &AsioBufferSwitchTimeInfo;
			oAsioCallback.sampleRateDidChange = &AsioSampleRateChanged;

			vector< ASIOBufferInfo > voBufferInfos( 4 );
			voBufferInfos[ 0 ].buffers;
			voBufferInfos[ 0 ].channelNum = 0;
			voBufferInfos[ 0 ].isInput = ASIOTrue;
			voBufferInfos[ 1 ].buffers;
			voBufferInfos[ 1 ].channelNum = 1;
			voBufferInfos[ 1 ].isInput = ASIOTrue;
			voBufferInfos[ 2 ].buffers;
			voBufferInfos[ 2 ].channelNum = 0;
			voBufferInfos[ 2 ].isInput = ASIOFalse;
			voBufferInfos[ 3 ].buffers;
			voBufferInfos[ 3 ].channelNum = 1;
			voBufferInfos[ 3 ].isInput = ASIOFalse;

			ASIOSampleRate fs;
			if( ASIOGetSampleRate( &fs ) == ASE_NotPresent )
				continue;

			long minSize, maxSize, preferredSize, granularity;
			ASIOError aeResult = ASIOGetBufferSize( &minSize, &maxSize, &preferredSize, &granularity );
			preferredSize = ( preferredSize == 0 ? min( 1024, maxSize ) : preferredSize );


			ae = ASIOCreateBuffers( &voBufferInfos.front(), 4, preferredSize, &oAsioCallback );
			assert( ae == ASE_OK );
						
			ae = ASIOStart();
			assert( ae == ASE_OK );

			VistaTimeUtils::Sleep( 2e3 );

			ae = ASIOStop();
			assert( ae == ASE_OK );

		}

		pDriver->removeCurrentDriver();
		delete pDriver;
	}

	return 0;
}
