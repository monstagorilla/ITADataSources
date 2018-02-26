#include "ITABufferDataSource.h"

#include <ITAException.h>
#include <ITANumericUtils.h>

ITABufferDatasource::ITABufferDatasource()
	: m_uiChannels( 0 )
	, m_uiBlocklength( 0 )
	, m_uiCapacity( 0 )
	, m_iCursor( 0 )
	, m_uiROIStart( 0 )
	, m_uiROIEnd( 0 )
	, m_uiNewROIStart( 0 )
	, m_uiNewROIEnd( 0 )
	, m_bLoopMode( false )
	, m_bNewLoopMode( false )
	, m_bChangeLoopMode( false )
	, m_bRewind( false )
	, m_bPaused( false )
	, m_bChangeROI( false )
	, m_bGetBlockPointerTouched( false )
	, m_bInit( false )
{
}

ITABufferDatasource::ITABufferDatasource( const float** ppfBuffer, unsigned int uiChannels, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode )
	: m_uiChannels( uiChannels )
	, m_uiBlocklength( uiBlocklength )
	, m_uiCapacity( 0 )
	, m_iCursor( 0 )
	, m_uiROIStart( 0 )
	, m_uiROIEnd( 0 )
	, m_uiNewROIStart( 0 )
	, m_uiNewROIEnd( 0 )
	, m_bLoopMode( bLoopMode )
	, m_bNewLoopMode( bLoopMode )
	, m_bChangeLoopMode( false )
	, m_bRewind( false )
	, m_bPaused( false )
	, m_bChangeROI( false )
	, m_bGetBlockPointerTouched( false )
	, m_bInit( false )
{
	// Initialisieren
	std::vector<float*> vpfBuffer;
	for( unsigned int i = 0; i < uiChannels; i++ )
		vpfBuffer.push_back( ( float* ) ppfBuffer[ i ] );

	Init( vpfBuffer, uiBuffersize, dSamplerate, uiBlocklength, bLoopMode );
}

ITABufferDatasource::ITABufferDatasource( const std::vector< float* >& vpfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode )
	: m_uiBlocklength( uiBlocklength )
	, m_uiCapacity( 0 )
	, m_iCursor( 0 )
	, m_uiROIStart( 0 )
	, m_uiROIEnd( 0 )
	, m_uiNewROIStart( 0 )
	, m_uiNewROIEnd( 0 )
	, m_bLoopMode( bLoopMode )
	, m_bNewLoopMode( bLoopMode )
	, m_bChangeLoopMode( false )
	, m_bRewind( false )
	, m_bPaused( false )
	, m_bChangeROI( false )
	, m_bGetBlockPointerTouched( false )
	, m_bInit( false )
{
	m_uiChannels = ( int ) vpfBuffer.size();

	// Initialisieren
	Init( vpfBuffer, uiBuffersize, dSamplerate, uiBlocklength, bLoopMode );
}

ITABufferDatasource::ITABufferDatasource( const float* pfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode )
	: m_uiChannels( 1 )
	, m_uiBlocklength( uiBlocklength )
	, m_uiCapacity( 0 )
	, m_iCursor( 0 )
	, m_uiROIStart( 0 )
	, m_uiROIEnd( 0 )
	, m_uiNewROIStart( 0 )
	, m_uiNewROIEnd( 0 )
	, m_bLoopMode( bLoopMode )
	, m_bNewLoopMode( bLoopMode )
	, m_bChangeLoopMode( false )
	, m_bRewind( false )
	, m_bPaused( false )
	, m_bChangeROI( false )
	, m_bGetBlockPointerTouched( false )
	, m_bInit( false )
{
	// Initialisieren
	std::vector<float*> vpfBuffer;
	vpfBuffer.push_back( ( float* ) pfBuffer );

	Init( vpfBuffer, uiBuffersize, dSamplerate, uiBlocklength, bLoopMode );
}

void ITABufferDatasource::Init( const std::vector<float*>& vpfBuffer, unsigned int uiBuffersize, double dSamplerate, unsigned int uiBlocklength, bool bLoopMode )
{
	if( ( vpfBuffer.empty() ) || ( uiBuffersize == 0 ) || ( dSamplerate <= 0 ) )
		ITA_EXCEPT0( INVALID_PARAMETER );

	m_dSamplerate = dSamplerate;
	m_uiChannels = ( unsigned int ) vpfBuffer.size();
	m_uiBlocklength = uiBlocklength;
	m_uiBuffersize = uiBuffersize;
	m_dSamplerate = dSamplerate;

	for( unsigned int i = 0; i < m_uiChannels; i++ )
	{
		// Keine Nullzeiger Kanalpuffer erlaubt
		if( vpfBuffer[ i ] == 0 ) ITA_EXCEPT0( INVALID_PARAMETER )
			m_vpfBuffer.push_back( vpfBuffer[ i ] );
	}

	// Initialen Loop-Modus setzen
	m_bLoopMode = m_bNewLoopMode = bLoopMode;

	// Kapazität = Nächstes kleineres Vielfaches der Blocklänge
	m_uiCapacity = lwrmulu( uiBuffersize, m_uiBlocklength );

	// Start-/Endcursor auf Gesamtlänge des Puffers einstellen.
	m_uiROIStart = m_uiNewROIStart = 0;
	m_uiROIEnd = m_uiNewROIEnd = m_uiCapacity;

	m_bInit = true;
}

unsigned int ITABufferDatasource::GetCapacity()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiCapacity kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return m_uiCapacity;
}

unsigned int ITABufferDatasource::GetAbsoluteCursor()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiCursor kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return ( unsigned int ) m_iCursor;
}

unsigned int ITABufferDatasource::GetCursor() {
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiCursor kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	/* TODO: Problem, falls Änderungen des Arbeitsbereiches, so dass
			 m_uiCursor plötzlich kleiner als m_uiNewROIStart */
	return ( unsigned int ) m_iCursor - m_uiNewROIStart;
}

void  ITABufferDatasource::SetCursor( unsigned int uiNewCursor )
{
	if( uiNewCursor > m_uiCapacity )
	{
		uiNewCursor = m_uiCapacity;
		return;
	}

	m_iCursor = ( int ) uiNewCursor;
}

void ITABufferDatasource::SetIsLooping( bool bEnabled )
{
	SetLoopMode( bEnabled );
}

bool ITABufferDatasource::GetIsLooping()
{
	return GetLoopMode();
}

bool ITABufferDatasource::GetLoopMode()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_bLoopMode kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return m_bNewLoopMode;
}

void ITABufferDatasource::SetLoopMode( bool bLoopMode )
{
	// Hinweis: Diese Implementierung ist Thread-Safe
	if( !m_bInit )
		return;

	m_bNewLoopMode = bLoopMode;
	m_bChangeLoopMode = true;
}


bool ITABufferDatasource::IsPaused() const
{
	return m_bPaused;
}

void ITABufferDatasource::SetPaused( bool bPaused )
{
	m_bPaused = bPaused;
}

void ITABufferDatasource::SetROI( unsigned int uiStartOffset, unsigned int uiEndOffset )
{
	// Hinweis: Diese Implementierung ist Thread-Safe
	if( !m_bInit ) return;

	// Parameter überprüfen
	if( ( uiStartOffset >= m_uiCapacity ) ||
		( uiEndOffset > m_uiCapacity ) ||
		( uiStartOffset > uiEndOffset ) ) ITA_EXCEPT0( INVALID_PARAMETER );

	// Nächstkleineres Blocklängen-Vielfaches des Endoffset bestimmen
	unsigned int l = lwrmulu( ( uiEndOffset - uiStartOffset ), m_uiBlocklength );
	m_uiNewROIStart = uiStartOffset;
	m_uiNewROIEnd = uiStartOffset + l;
	m_bChangeROI = true;
}

unsigned int ITABufferDatasource::GetROIStart()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiStartCursor kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return m_uiNewROIStart;
}


unsigned int ITABufferDatasource::GetROIEnd()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiEndCursor kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return m_uiNewROIEnd;
}

unsigned int ITABufferDatasource::GetROILength()
{
	/* TODO: Diese Implementierung ist NICHT THREAD-SAFE.
			 m_uiEndCursor kann durch IncrementReadPointer modifiziert werden,
			 während der Wert hier ausgelesen wird. */
	return m_uiNewROIEnd - m_uiNewROIStart;
}

void ITABufferDatasource::Rewind()
{
	if( !m_bInit )
		return;

	m_bRewind = true;
	Update();
}

unsigned int ITABufferDatasource::GetBlocklength() const
{
	return m_uiBlocklength;
}

unsigned int ITABufferDatasource::GetNumberOfChannels() const
{
	return m_uiChannels;
}

double ITABufferDatasource::GetSampleRate() const
{
	return m_dSamplerate;
}

const float* ITABufferDatasource::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
{
	if( !m_bInit )
		return NULL;

	// First call?
	if( m_bGetBlockPointerTouched == false )
	{
		bool bIncRequired = Update();
		m_bPausedInternal = m_bPaused;
		m_bGetBlockPointerTouched = true;
	}

	if( uiChannel >= m_uiChannels )
		return NULL;

	// Keine Eingangsdaten in der Datei oder keine Wiederholung und Ende erreicht:
	// Dann Nullzeiger zurückgeben (d.h. keine Daten vorhanden)
	if( ( unsigned int )
		m_iCursor >= m_uiROIEnd ) return 0;

	// Pausiert? => Dann auch Nullzeiger zurückgeben
	if( m_bPausedInternal )
		return 0;

	// Sonst Datenzeiger zurückgeben
	return( m_vpfBuffer[ uiChannel ] + ( unsigned int ) m_iCursor );	// übergebenen Cursor verwenden
}

void ITABufferDatasource::IncrementBlockPointer()
{
	if( !m_bInit )
		return;

	if( !Update() )
		return;

	if( m_bPausedInternal )
	{
		m_bGetBlockPointerTouched = false;
		return;
	}

	// Ganz normale Verhaltensweise:
	if( ( unsigned int ) m_iCursor < m_uiROIEnd )
		m_iCursor = m_iCursor + ( int ) m_uiBlocklength;
	if( m_bLoopMode && ( ( unsigned int ) ( m_iCursor ) >= m_uiROIEnd ) )
		m_iCursor = int( m_uiROIStart );

	m_bGetBlockPointerTouched = false;
}

bool ITABufferDatasource::Update()
{
	if( m_bChangeLoopMode )
	{
		m_bLoopMode = m_bNewLoopMode;
		m_bChangeLoopMode = false;
	}

	if( m_bChangeROI )
	{
		m_uiROIStart = m_uiNewROIStart;
		m_uiROIEnd = m_uiNewROIEnd;
		m_bChangeROI = false;

		if( ( m_uiNewROIStart > ( unsigned int ) m_iCursor ) || ( m_uiNewROIEnd < ( unsigned int ) m_iCursor ) )
			// Der aktuelle Cursor liegt außerhalb des Arbeitsbereiches
			if( ( m_bLoopMode ) || ( ( unsigned int ) m_iCursor < m_uiNewROIStart ) )
			{
				// Falls Wiederholung eingeschaltet ist oder der Anfang des 
				// neuen Arbeitsbereiches vor den aktuellen Cursor verschoben
				// wurde, so springt der Cursor nun an den Anfang des neuen
				// Arbeitsbereiches
				m_iCursor = ( int ) m_uiROIStart;

				// Ein Rückspulwunsch ist damit erfüllt
				m_bRewind = false;
				return false;
			}
	}

	if( m_bRewind )
	{
		m_iCursor = ( int ) m_uiROIStart;
		m_bRewind = false;
		return false;
	}

	// Wiederholungsmodus eingeschaltet und Cursor bereits hinter dem Ende
	// des Arbeitsbereiches? Dann wieder zum Beginn des Arbeitsbereiches springen
	if( m_bLoopMode && ( ( unsigned int ) m_iCursor >= m_uiROIEnd ) )
	{
		m_iCursor = ( int ) m_uiROIStart;
		return false;
	}

	// Aufrufer müssen den Cursor noch selber inkrementieren
	return true;
}
