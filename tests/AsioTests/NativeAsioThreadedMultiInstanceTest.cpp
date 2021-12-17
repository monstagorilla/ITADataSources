#include <VistaBase/VistaTimeUtils.h>
#include <VistaInterProcComm/Concurrency/VistaThread.h>
#include <algorithm>
#include <cassert>
#include <common/asio.h>
#include <conio.h>
#include <host/asiodrivers.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Errors are validated ussing assertions. Run this in debug, only.

using namespace std;

AsioDriverList g_oAsioDriverList;

class CThreadedASIO;
typedef map<long, CThreadedASIO*> CThreadedASIORegistry;
CThreadedASIORegistry g_oRegistry;

class CThreadedASIO : public VistaThread
{
public:
	inline CThreadedASIO( const int iDriverIndex ) : m_iDriverIndex( iDriverIndex )
	{
		ASIOError ae;

		assert( m_iDriverIndex < g_oAsioDriverList.asioGetNumDev( ) && m_iDriverIndex >= 0 );
		m_sDriverName.resize( 256 );
		m_oDriver.asioGetDriverName( m_iDriverIndex, &m_sDriverName[0], int( m_sDriverName.size( ) ) );
		m_sDriverName = std::string( m_sDriverName.c_str( ) ); // Strip

		cout << "[" << m_sDriverName << "] Initializing ASIO from thread " << VistaThread::GetThreadIdentity( ) << endl;

		VistaTimeUtils::Sleep( 1000 );

		bool bLoadSuccess = m_oDriver.loadDriver( &m_sDriverName[0] );
		assert( bLoadSuccess );

		VistaTimeUtils::Sleep( 1000 );

		ASIODriverInfo oDriverInfo;
		oDriverInfo.asioVersion = 2;
		assert( ( ae = ASIOInit( &oDriverInfo ) ) == ASE_OK );

		ASIOCallbacks oAsioCallback;
		oAsioCallback.asioMessage          = &( CThreadedASIO::AsioMessages );
		oAsioCallback.bufferSwitch         = &( CThreadedASIO::AsioBufferSwitch );
		oAsioCallback.bufferSwitchTimeInfo = &( CThreadedASIO::AsioBufferSwitchTimeInfo );
		oAsioCallback.sampleRateDidChange  = &( CThreadedASIO::AsioSampleRateChanged );

		vector<ASIOBufferInfo> voBufferInfos( 4 );
		voBufferInfos[0].buffers;
		voBufferInfos[0].channelNum = 0;
		voBufferInfos[0].isInput    = ASIOTrue;
		voBufferInfos[1].buffers;
		voBufferInfos[1].channelNum = 1;
		voBufferInfos[1].isInput    = ASIOTrue;
		voBufferInfos[2].buffers;
		voBufferInfos[2].channelNum = 0;
		voBufferInfos[2].isInput    = ASIOFalse;
		voBufferInfos[3].buffers;
		voBufferInfos[3].channelNum = 1;
		voBufferInfos[3].isInput    = ASIOFalse;

		ASIOSampleRate fs;
		assert( ASIOGetSampleRate( &fs ) == ASE_OK );

		long minSize, maxSize, preferredSize, granularity;
		ASIOError aeResult = ASIOGetBufferSize( &minSize, &maxSize, &preferredSize, &granularity );
		preferredSize      = ( preferredSize == 0 ? min( 1024, maxSize ) : preferredSize );

		assert( ASE_OK == ASIOCreateBuffers( &voBufferInfos.front( ), 4, preferredSize, &oAsioCallback ) );

		g_oRegistry[VistaThread::GetThreadIdentity( )] = this;

		assert( ASE_OK == ASIOStart( ) );
		cout << "[" << m_sDriverName << "] Started ASIO from thread " << VistaThread::GetThreadIdentity( ) << endl;
		VistaTimeUtils::Sleep( 10000 );
		assert( ASE_OK == ASIOStop( ) );
	};

	inline void ThreadBody( ) { };

	inline static ASIOTime* AsioBufferSwitchTimeInfo( ASIOTime* timeInfo, long index, ASIOBool processNow ) { return timeInfo; };

	inline static void AsioBufferSwitch( long index, ASIOBool processNow )
	{
		ASIOTime timeInfo;
		memset( &timeInfo, 0, sizeof( timeInfo ) );

		if( ASIOGetSamplePosition( &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime ) == ASE_OK )
			timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

		AsioBufferSwitchTimeInfo( &timeInfo, index, processNow );

		long iThreadID = VistaThread::GetCallingThreadIdentity( );
		cout << "Buffer switch called from thread " << iThreadID << endl;
		// g_oRegistry[ iThreadID ];
	};

	inline static void AsioSampleRateChanged( ASIOSampleRate fs )
	{
		cout << "Sample rate changed to " << fs << endl;
		return;
	};

	inline static long AsioMessages( long selector, long value, void*, double* ) { return 0; };

private:
	int m_iDriverIndex;
	std::string m_sDriverName;
	AsioDrivers m_oDriver;
};


int main( int, char[] )
{
	cout << "Main thread is called by " << VistaThread::GetCallingThreadIdentity( ) << endl;
	CThreadedASIO oAsio1( 1 );
	// CThreadedASIO oAsio2( 1 );

	oAsio1.Run( );
	// oAsio2.Run();

	VistaTimeUtils::Sleep( 10000 );

	return 0;
}
