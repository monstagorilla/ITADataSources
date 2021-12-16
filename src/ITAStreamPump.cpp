#include <ITAClock.h>
#include <ITADataSource.h>
#include <ITAStreamPump.h>

ITAStreamPump::ITAStreamPump( ITADatasource* pDatasource ) : m_pDatasource( pDatasource ), m_pTimer( NULL )
{
	m_pTimer = new ITATimer( (double)pDatasource->GetBlocklength( ) / pDatasource->GetSampleRate( ), true );
	m_pTimer->attach( this );
	m_uiChannels = pDatasource->GetNumberOfChannels( );
}

ITAStreamPump::~ITAStreamPump( )
{
	delete m_pTimer;
}

void ITAStreamPump::StartStreaming( )
{
	m_pTimer->start( );
}

void ITAStreamPump::StopStreaming( )
{
	m_pTimer->stop( );
}

void ITAStreamPump::handleTimerEvent( const ITATimer& tSource )
{
	if( &tSource == m_pTimer )
	{
		for( unsigned int i = 0; i < m_uiChannels; i++ )
			m_pDatasource->GetBlockPointer( i, &m_siState );
		m_pDatasource->IncrementBlockPointer( );

		m_siState.nSamples += m_pDatasource->GetBlocklength( );
		m_siState.dStreamTimeCode = (double)( m_siState.nSamples ) / m_pDatasource->GetSampleRate( );
		m_siState.dSysTimeCode    = ITAClock::getDefaultClock( )->getTime( );
	}
}
