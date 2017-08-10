#include <cassert>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <host/asiodrivers.h>
#include <common/asio.h>

#include <VistaBase/VistaTimeUtils.h>

using namespace std;

ASIOTime* AsioBufferSwitchTimeInfoA( ASIOTime *timeInfo, long, ASIOBool )
{
	return timeInfo;
};

void AsioBufferSwitchA( long nIndex, ASIOBool processNow )
{
	ASIOTime timeInfo;
	memset( &timeInfo, 0, sizeof( timeInfo ) );

	if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	AsioBufferSwitchTimeInfoA( &timeInfo, nIndex, processNow );
};

void AsioSampleRateChangedA( ASIOSampleRate fs )
{
	cout << "Sample rate changed to " << fs << endl;
	return;
};

long AsioMessagesA( long, long, void*, double* )
{
	return 0;
};

ASIOTime* AsioBufferSwitchTimeInfoB( ASIOTime *timeInfo, long, ASIOBool )
{
	return timeInfo;
};

void AsioBufferSwitchB( long nIndex, ASIOBool processNow )
{
	ASIOTime timeInfo;
	memset( &timeInfo, 0, sizeof( timeInfo ) );

	if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	AsioBufferSwitchTimeInfoB( &timeInfo, nIndex, processNow );
};

void AsioSampleRateChangedB( ASIOSampleRate fs )
{
	cout << "Sample rate changed to " << fs << endl;
	return;
};

long AsioMessagesB( long, long, void*, double* )
{
	return 0;
};


int main( int, char[] )
{
	ASIOError ae;
	AsioDriverList* pDrivers = new AsioDriverList();
	long nNumDrivers = pDrivers->asioGetNumDev();
	cout << "Number of ASIO drivers: " << nNumDrivers << endl;
	delete pDrivers;

	// Driver A

	int iDriverIndexA = 1;
	AsioDrivers* pDriverA = new AsioDrivers();

	string sDriverNameA;
	sDriverNameA.resize( 256 );
	pDriverA->asioGetDriverName( iDriverIndexA, &sDriverNameA[ 0 ], int( sDriverNameA.size() ) );
	sDriverNameA = std::string( sDriverNameA.c_str() ); // Strip
	cout << "Driver name: " << sDriverNameA << "." << endl;


	int iDriverIndexB = 1;
	//AsioDrivers* pDriverB = new AsioDrivers();
	string sDriverNameB;
	sDriverNameB.resize( 256 );
	pDriverA->asioGetDriverName( iDriverIndexB, &sDriverNameB[ 0 ], int( sDriverNameB.size() ) );
	sDriverNameB = std::string( sDriverNameB.c_str() ); // Strip
	cout << "Driver name: " << sDriverNameB << "." << endl;

	bool bLoadSuccessA = pDriverA->loadDriver( &sDriverNameA[ 0 ] );
	assert( bLoadSuccessA );

	bool bLoadSuccessB = pDriverA->loadDriver( &sDriverNameB[ 0 ] );
	assert( bLoadSuccessB );
				
	ASIODriverInfo oDriverInfo;
	oDriverInfo.asioVersion = 2;
	if( ASIOInit( &oDriverInfo ) != ASE_OK )
	{
		cout << "Could not initialize " << sDriverNameA << ", skipping." << endl;
		return 255;
	}

	ASIOCallbacks oAsioCallbackA;
	oAsioCallbackA.asioMessage = &AsioMessagesA;
	oAsioCallbackA.bufferSwitch = &AsioBufferSwitchA;
	oAsioCallbackA.bufferSwitchTimeInfo = &AsioBufferSwitchTimeInfoA;
	oAsioCallbackA.sampleRateDidChange = &AsioSampleRateChangedA;

	vector< ASIOBufferInfo > voBufferInfosA( 4 );
	voBufferInfosA[ 0 ].buffers;
	voBufferInfosA[ 0 ].channelNum = 0;
	voBufferInfosA[ 0 ].isInput = ASIOTrue;
	voBufferInfosA[ 1 ].buffers;
	voBufferInfosA[ 1 ].channelNum = 1;
	voBufferInfosA[ 1 ].isInput = ASIOTrue;
	voBufferInfosA[ 2 ].buffers;
	voBufferInfosA[ 2 ].channelNum = 0;
	voBufferInfosA[ 2 ].isInput = ASIOFalse;
	voBufferInfosA[ 3 ].buffers;
	voBufferInfosA[ 3 ].channelNum = 1;
	voBufferInfosA[ 3 ].isInput = ASIOFalse;

	ASIOSampleRate fs;
	if( ASIOGetSampleRate( &fs ) == ASE_NotPresent )
		return 255;

	long minSize, maxSize, preferredSize, granularity;
	ae = ASIOGetBufferSize( &minSize, &maxSize, &preferredSize, &granularity );
	preferredSize = ( preferredSize == 0 ? min( 1024, maxSize ) : preferredSize );

	ae = ASIOCreateBuffers( &voBufferInfosA.front(), 4, preferredSize, &oAsioCallbackA );
	assert( ae == ASE_OK );
						
	ae = ASIOStart();
	assert( ae == ASE_OK );

	VistaTimeUtils::Sleep( int( 2e3 ) );

	ae = ASIOStop();
	assert( ae == ASE_OK );

	
	// Driver B

	ASIODriverInfo oDriverInfoB;
	oDriverInfoB.asioVersion = 2;
	if( ASIOInit( &oDriverInfoB ) != ASE_OK )
	{
		cout << "Could not initialize " << sDriverNameB << ", skipping." << endl;
		return 255;
	}

	ASIOCallbacks oAsioCallbackB;
	oAsioCallbackB.asioMessage = &AsioMessagesB;
	oAsioCallbackB.bufferSwitch = &AsioBufferSwitchB;
	oAsioCallbackB.bufferSwitchTimeInfo = &AsioBufferSwitchTimeInfoB;
	oAsioCallbackB.sampleRateDidChange = &AsioSampleRateChangedB;

	vector< ASIOBufferInfo > voBufferInfosB( 4 );
	voBufferInfosB[ 0 ].buffers;
	voBufferInfosB[ 0 ].channelNum = 0;
	voBufferInfosB[ 0 ].isInput = ASIOTrue;
	voBufferInfosB[ 1 ].buffers;
	voBufferInfosB[ 1 ].channelNum = 1;
	voBufferInfosB[ 1 ].isInput = ASIOTrue;
	voBufferInfosB[ 2 ].buffers;
	voBufferInfosB[ 2 ].channelNum = 0;
	voBufferInfosB[ 2 ].isInput = ASIOFalse;
	voBufferInfosB[ 3 ].buffers;
	voBufferInfosB[ 3 ].channelNum = 1;
	voBufferInfosB[ 3 ].isInput = ASIOFalse;

	if( ASIOGetSampleRate( &fs ) == ASE_NotPresent )
		return 255;

	ae = ASIOGetBufferSize( &minSize, &maxSize, &preferredSize, &granularity );
	preferredSize = ( preferredSize == 0 ? min( 1024, maxSize ) : preferredSize );


	ae = ASIOCreateBuffers( &voBufferInfosB.front(), 4, preferredSize, &oAsioCallbackB );
	assert( ae == ASE_OK );

	ae = ASIOStart();
	assert( ae == ASE_OK );
	
	VistaTimeUtils::Sleep( int( 2e3 ) );

	ae = ASIOStop();
	assert( ae == ASE_OK );

	pDriverA->removeCurrentDriver();
	delete pDriverA;

//	pDriverB->removeCurrentDriver();
	//delete pDriverB;
	
	return 0;
}
