#include <cassert>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include <host/asiodrivers.h>
#include <common/asio.h>

#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/Concurrency/VistaThread.h>

// Errors are validated ussing assertions. Run this in debug, only.

using namespace std;

AsioDriverList g_oAsioDriverList;

class CThreadedASIO;
typedef map< long, CThreadedASIO* > CThreadedASIORegistry;
CThreadedASIORegistry g_oRegistry;

class CThreadedASIO : public VistaThread
{
public:
	inline CThreadedASIO( const int iDriverIndex )
		: m_iDriverIndex( iDriverIndex )
	{
		assert( iDriverIndex < g_oAsioDriverList.asioGetNumDev() && iDriverIndex > 0 );
		m_sDriverName.resize( 256 );
		m_oDriver.asioGetDriverName( iDriverIndex, &m_sDriverName[ 0 ], int( m_sDriverName.size() ) );
		m_sDriverName = std::string( m_sDriverName.c_str() ); // Strip
		cout << "New threaded ASIO instance with driver name: " << m_sDriverName << endl;

		bool bLoadSuccess = m_oDriver.loadDriver( &m_sDriverName[ 0 ] );

		ASIODriverInfo oDriverInfo;
		oDriverInfo.asioVersion = 2;
		assert( ASIOInit( &oDriverInfo ) == ASE_OK );

		ASIOCallbacks oAsioCallback;
		//long AsioMessages( long selector, long value, void*, double* )
		
		oAsioCallback.asioMessage = long( CThreadedASIO::* )( long, long, void*, double* );
		oAsioCallback.bufferSwitch = &( CThreadedASIO::AsioBufferSwitch );
		oAsioCallback.bufferSwitchTimeInfo = &( CThreadedASIO::AsioBufferSwitchTimeInfo );
		oAsioCallback.sampleRateDidChange = &( CThreadedASIO::AsioSampleRateChanged );

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
		assert( ASIOGetSampleRate( &fs ) == ASE_OK );

		long minSize, maxSize, preferredSize, granularity;
		ASIOError aeResult = ASIOGetBufferSize( &minSize, &maxSize, &preferredSize, &granularity );
		preferredSize = ( preferredSize == 0 ? min( 1024, maxSize ) : preferredSize );

		assert( ASE_OK == ASIOCreateBuffers( &voBufferInfos.front(), 4, preferredSize, &oAsioCallback ) );

		g_oRegistry[ VistaThread::GetThreadIdentity() ] = this;
	};

	inline void StartStreaming()
	{
		assert( ASE_OK == ASIOStart() );
	};

	inline void ThreadBody()
	{
		cout << "[" << m_sDriverName << "] Tic Toc" << endl;
		VistaTimeUtils::Sleep( 1000 );
	};

	inline void StopStreaming()
	{
		assert( ASE_OK == ASIOStop() );
	};

	inline static ASIOTime* AsioBufferSwitchTimeInfo( ASIOTime *timeInfo, long index, ASIOBool processNow )
	{
		return timeInfo;
	};
	
	inline static void AsioBufferSwitch( long index, ASIOBool processNow )
	{
		ASIOTime timeInfo;
		memset( &timeInfo, 0, sizeof( timeInfo ) );

		if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
			timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

		AsioBufferSwitchTimeInfo( &timeInfo, index, processNow );

		long iThreadID = VistaThread::GetThreadIdentity();
		g_oRegistry[ iThreadID ];
	};

	inline static void AsioSampleRateChanged( ASIOSampleRate fs )
	{
		cout << "Sample rate changed to " << fs << endl;
		return;
	};
	
	inline long AsioMessages( long selector, long value, void*, double* )
	{
		return 0;
	};

private:
	int m_iDriverIndex;
	std::string m_sDriverName;
	AsioDrivers m_oDriver;
};


int main( int, char[] )
{
	CThreadedASIO oAsio1( 2 );
	CThreadedASIO oAsio2( 2 );

	oAsio1.StartStreaming();
	oAsio2.StartStreaming();

	VistaTimeUtils::Sleep( 3000 );

	oAsio1.StopStreaming();
	oAsio2.StopStreaming();

	return 0;
}
