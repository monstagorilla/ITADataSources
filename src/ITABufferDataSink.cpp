#include <ITABufferDataSink.h>
#include <ITAClock.h>
#include <ITADataSource.h>
#include <ITAException.h>
#include <ITAFastMath.h>
#include <ITAFunctors.h>
#include <ITANumericUtils.h>
#include <algorithm>

#ifndef WIN32
#	include <memory.h>
#endif

ITABufferDataSink::ITABufferDataSink( ITADatasource* pdsSource, int iBuffersize )
    : m_pdsSource( pdsSource )
    , m_vpfBuffer( pdsSource->GetNumberOfChannels( ), NULL )
    , m_vpfBufferX( pdsSource->GetNumberOfChannels( ), NULL )
    , m_uiBuffersize( iBuffersize )
    , m_bManagedBuffer( true )
{
	// Puffer erzeugen
	for( unsigned int i = 0; i < pdsSource->GetNumberOfChannels( ); i++ )
	{
		m_vpfBuffer[i]  = fm_falloc( iBuffersize, true );
		m_vpfBufferX[i] = (const float*)m_vpfBuffer[i];
	}

	m_uiWriteCursor = 0;
}

ITABufferDataSink::ITABufferDataSink( ITADatasource* pdsSource, std::vector<float*> vpfBuffer, int iBuffersize )
    : m_pdsSource( pdsSource )
    , m_vpfBuffer( vpfBuffer )
    , m_vpfBufferX( pdsSource->GetNumberOfChannels( ), NULL )
    , m_uiBuffersize( iBuffersize )
    , m_bManagedBuffer( false )
{
	// Parameter prüfen
	if( pdsSource->GetNumberOfChannels( ) != (unsigned int)vpfBuffer.size( ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Number of buffers does not meet the datasource's number of channels" );

	// Puffer erzeugen
	for( unsigned int i = 0; i < pdsSource->GetNumberOfChannels( ); i++ )
		m_vpfBufferX[i] = (const float*)m_vpfBuffer[i];

	m_uiWriteCursor = 0;
}

ITABufferDataSink::~ITABufferDataSink( )
{
	if( m_bManagedBuffer )
		std::for_each( m_vpfBuffer.begin( ), m_vpfBuffer.end( ), fm_free );
}

int ITABufferDataSink::GetBuffersize( ) const
{
	return m_uiBuffersize;
}

std::vector<float*> ITABufferDataSink::GetBuffers( ) const
{
	return m_vpfBuffer;
}

unsigned int ITABufferDataSink::GetWriteCursor( ) const
{
	return m_uiWriteCursor;
}

void ITABufferDataSink::SetWriteCursor( unsigned int uiWriteCursor )
{
	m_uiWriteCursor = ( std::min )( uiWriteCursor, m_uiBuffersize );
}

void ITABufferDataSink::Transfer( unsigned int uiSamples )
{
	/*
	 *  Hinweis: Auch wenn der Puffer schon vollgeschrieben wurde,
	 *           so werden trotzdem die Daten von der Datenquelle
	 *           geholt und dann ins Nirvana geworfen...
	 */

	if( !m_pdsSource )
		return;

	unsigned int n = uprdivu( uiSamples, m_pdsSource->GetBlocklength( ) ); // Number of block increments until data is completely transferred

	// Iterate over blocks
	for( unsigned int i = 0; i < n; i++ )
	{
		for( int j = 0; j < int( m_pdsSource->GetNumberOfChannels( ) ); j++ )
		{
			const float* pfSrc = m_pdsSource->GetBlockPointer( j, &m_siState );
			unsigned int m     = ( std::min )( m_uiBuffersize - m_uiWriteCursor, m_pdsSource->GetBlocklength( ) ); // One sample block or less
			if( m > 0 )
			{
				if( pfSrc )
					memcpy( m_vpfBuffer[j] + m_uiWriteCursor, pfSrc, m * sizeof( float ) );
				else
					memset( m_vpfBuffer[j] + m_uiWriteCursor, 0, m * sizeof( float ) );
			}

			m_uiWriteCursor += m;
			m_siState.nSamples += m_pdsSource->GetBlocklength( );
			m_siState.dStreamTimeCode = (double)( m_siState.nSamples ) / m_pdsSource->GetSampleRate( );
			m_siState.dSysTimeCode    = ITAClock::getDefaultClock( )->getTime( );

			m_pdsSource->IncrementBlockPointer( );
		}
	}
}
