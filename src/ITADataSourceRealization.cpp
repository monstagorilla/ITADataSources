#include "ITADataSourceRealization.h"
#include <ITAFastMath.h>
#include <cassert>

ITADatasourceRealization::ITADatasourceRealization( unsigned int uiChannels, double dSamplerate, unsigned int uiBlocklength, unsigned int uiCapacity )
{
	assert( dSamplerate > 0 );
	m_dSampleRate = dSamplerate;
	m_oStreamProps.dSamplerate = dSamplerate;

	Init( uiChannels, uiBlocklength, uiCapacity );
}

void ITADatasourceRealization::Init( unsigned int uiChannels, unsigned int uiBlocklength, unsigned int uiCapacity )
{
	assert( uiChannels > 0 );
	assert( uiBlocklength > 0 );
	assert( uiCapacity > 0 );

	// Interne Felder setzen

	/*
	 *  Hinweis: m_uiBufferSize ist nicht die Länge des gesamten Puffers,
	 *           sondern nur die Länge der Blöcke EINES Kanals. Des Weiteren
	 *           ist der Puffer ein Element größer als die Kapazität, damit
	 *           Anzahl Kapazität Blöcke vorausgespeichert werden können.
	 */

	m_uiChannels = uiChannels;
	m_uiBlocklength = uiBlocklength;

	m_oStreamProps.dSamplerate = m_dSampleRate;
	m_oStreamProps.uiChannels = m_uiChannels;
	m_oStreamProps.uiBlocklength = m_uiBlocklength;

	m_uiBufferSize = uiBlocklength * ( uiCapacity + 1 );

	m_pEventHandler = NULL;

	/*
	   Organisation des Puffers: Damit die Blöcke der einzelnen Kanäle
	   im Speicher ortlich näher liegen ist das Array wiefolgt indiziert:

	   [1. Block Kanal 1], ..., [1. Block Kanal k], [2. Block Kanal 1], ...

	   */

	// Puffer erzeugen und mit Nullen initialiseren
	// TODO: Fehlerbehandlung beim Speicherallozieren
	/* Bugfix zu Bug #001:

	   Hier wurde der Puffer einfach um 1024 Felder verlängert.
	   Damit Funktioniert Wuschels ASIO4ALL jetzt. Ungeklärt aber
	   warum der Fehler auftrat?

	   2005-2-14
	   */

	m_pfBuffer = fm_falloc( m_uiBufferSize * m_uiChannels + /* >>> */ 1024 /* <<< */, false );

	Reset();
}

ITADatasourceRealization::~ITADatasourceRealization()
{
	fm_free( m_pfBuffer );
}

void ITADatasourceRealization::Reset()
{
	m_uiReadCursor = 0;
	m_uiWriteCursor = 0;

	// Fehler-Indikatoren zurücksetzen
	m_iBufferUnderflows = 0;
	m_iBufferOverflows = 0;
	m_iGBPReentrances = 0;

	m_iGBPEntrances = 0;
	m_bGBPFirst = true;

	fm_zero( m_pfBuffer, m_uiBufferSize * m_uiChannels + /* >>> */ 1024 /* <<< */ );
}

bool ITADatasourceRealization::HasStreamErrors() const
{
	return ( m_iBufferUnderflows > 0 ) || ( m_iBufferOverflows > 0 ) || ( m_iGBPReentrances > 0 );
}

ITADatasourceRealizationEventHandler* ITADatasourceRealization::GetStreamEventHandler() const
{
	return m_pEventHandler;
}

void ITADatasourceRealization::SetStreamEventHandler( ITADatasourceRealizationEventHandler* pHandler )
{
	m_pEventHandler = pHandler;
}

const float* ITADatasourceRealization::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
{
	assert( uiChannel < m_uiChannels );

	/*
	 * Parallele Wiedereintritte in GBP bedeutet, das die Rechenzeit überschritten wurde
	 * und die Audio-Hardware bereits die nächste Runde im Stream eingeläutet hat.
	 *
	 * WICHTIG: Dies sollte nicht passieren. Fehler beim anwendenden Programmierer!
	 */
	if( ++m_iGBPEntrances > 1 )
	{
		--m_iGBPEntrances;
		++m_iGBPReentrances;
		return NULL;
	}

	// Hook/Handler aufrufen
	PreGetBlockPointer();
	if( m_pEventHandler )
		m_pEventHandler->HandlePreGetBlockPointer( this, uiChannel );

	if( m_bGBPFirst )
	{
		// Erster Eintritt in GBP seit letztem IBP => Daten produzieren
		ProcessStream( pStreamInfo );

		if( m_pEventHandler )
			m_pEventHandler->HandleProcessStream( this, pStreamInfo );

		m_bGBPFirst = false;
	}

	/*
	 *  Semantik: Daten im Puffer <=> Read cursor != write cursor
	 *
	 *  Beim letzen Aufruf von ProcessStream wurden keine weiteren Daten in
	 *  den Ausgabepuffer gelegt und nun ist dieser Leer. Daher Nullzeiger zurückgeben.
	 *
	 *  WICHTIG: Dies sollte nicht passieren. Fehler beim anwendenden Programmierer!
	 */

	unsigned int uiLocalReadCursor = m_uiReadCursor;
	if( uiLocalReadCursor == m_uiWriteCursor )
	{
		++m_iBufferUnderflows;
		--m_iGBPEntrances;
		return NULL;
	}

	--m_iGBPEntrances;
	return m_pfBuffer + ( uiChannel * m_uiBufferSize ) + uiLocalReadCursor;
}

void ITADatasourceRealization::IncrementBlockPointer()
{
	unsigned int uiLocalReadCursor = m_uiReadCursor;

	if( uiLocalReadCursor == m_uiWriteCursor )
		// Keine Daten im Ausgabepuffer? Kein Inkrement möglich! (Fehlerfall)
		++m_iBufferUnderflows;
	else
		// Lesezeiger inkrementieren
		m_uiReadCursor = ( uiLocalReadCursor + m_uiBlocklength ) % m_uiBufferSize;

	m_bGBPFirst = true;

	PostIncrementBlockPointer();

	if( m_pEventHandler )
		m_pEventHandler->HandlePostIncrementBlockPointer( this );
}

float* ITADatasourceRealization::GetWritePointer( unsigned int uiChannel )
{
	assert( uiChannel < m_uiChannels );
	return m_pfBuffer + ( uiChannel * m_uiBufferSize ) + m_uiWriteCursor;
}

void ITADatasourceRealization::IncrementWritePointer()
{
	// Lokaler Schreibcursor
	unsigned int uiLocalWriteCursor = m_uiWriteCursor;
	unsigned int uiNewWriteCursor = ( uiLocalWriteCursor + m_uiBlocklength ) % m_uiBufferSize;

	// Pufferüberlauf
	if( uiNewWriteCursor == m_uiReadCursor )
	{
		++m_iBufferOverflows;
		return;
	}

	// Neuen Schreibcursor setzen
	m_uiWriteCursor = uiNewWriteCursor;
}

void ITADatasourceRealizationEventHandler::HandlePreGetBlockPointer( ITADatasourceRealization*, unsigned int ) {}
void ITADatasourceRealizationEventHandler::HandlePostIncrementBlockPointer( ITADatasourceRealization* ) {}
void ITADatasourceRealizationEventHandler::HandleProcessStream( ITADatasourceRealization*, const ITAStreamInfo* ) {}
