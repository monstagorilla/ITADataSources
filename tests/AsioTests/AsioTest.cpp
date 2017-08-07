#include <cassert>
#include <conio.h>
#include <iostream>
#include <string>

#include <host/asiodrivers.h>
#include "common/asio.h"

using namespace std;

ASIOCallbacks oAsioCallback;

void AsioBufferSwitch( long index, ASIOBool processNow )
{
	ASIOTime timeInfo;
	memset( &timeInfo, 0, sizeof( timeInfo ) );

	if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	// @todo:
	//bufferSwitchTimeInfo( &timeInfo, index, processNow );
};

void AsioSampleRateChanged( ASIOSampleRate ) {};

long AsioMessages( long selector, long value, void*, double* ) { return 0; };

ASIOTime *AsioBufferSwitchTimeInfo( ASIOTime *timeInfo, long index, ASIOBool processNow )
{
	// @todo:
	/*
	for( int i = 0; i < oAsioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++ )
	{
		if( asioDriverInfo.bufferInfos[ i ].isInput == TRUE )
		{
			assert( asioDriverInfo.channelInfos[ i ].type == ASIOSTInt24LSB );
		}
	}
	*/

	return nullptr;
};

int main( int argc, char* argv[] )
{
	AsioDriverList* pDrivers = new AsioDrivers();

	oAsioCallback.bufferSwitch = &AsioBufferSwitch;
	oAsioCallback.sampleRateDidChange = &AsioSampleRateChanged;
	oAsioCallback.asioMessage = &AsioMessages;
	oAsioCallback.bufferSwitchTimeInfo = &AsioBufferSwitchTimeInfo;

	long lNumDrivers = pDrivers->asioGetNumDev();

	if( lNumDrivers > 0 )
	{
		cout << "Found ASIO drivers:" << endl << endl;
		std::string sDriverName;
		sDriverName.reserve( 255 );
		for( long i = 0; i < lNumDrivers; i++ )
		{
			pDrivers->asioGetDriverName( i, &sDriverName[ 0 ], 255 );
			cout << "[" << i + 1 << "] \"" << sDriverName << "\"" << endl;
		}
	}
	else
		cout << "No ASIO driver found" << endl;

	return 0;
}
