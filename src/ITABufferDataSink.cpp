#include <ITABufferDataSink.h>

#include <algorithm>
#include <ITAFastMath.h>
#include <ITADataSource.h>
#include <ITAException.h>
#include <ITAFunctors.h>
#include <ITANumericUtils.h>

#ifndef WIN32
#include <memory.h>
#endif

ITABufferDatasink::ITABufferDatasink( ITADatasource* pdsSource, unsigned int uiBuffersize )
: m_pdsSource( pdsSource )
, m_vpfBuffer( pdsSource->GetNumberOfChannels(), NULL )
, m_vpfBufferX( pdsSource->GetNumberOfChannels(), NULL )
, m_uiBuffersize( uiBuffersize )
, m_bManagedBuffer( true )
{
	// Puffer erzeugen
	for (unsigned int i=0; i<pdsSource->GetNumberOfChannels(); i++) {
		m_vpfBuffer[i] = fm_falloc(uiBuffersize, true);
		m_vpfBufferX[i] = (const float*) m_vpfBuffer[i];
	}

	m_uiWriteCursor = 0;
}

ITABufferDatasink::ITABufferDatasink( ITADatasource* pdsSource, std::vector<float*> vpfBuffer, unsigned int uiBuffersize )
: m_pdsSource( pdsSource )
, m_vpfBuffer( vpfBuffer )
, m_vpfBufferX( pdsSource->GetNumberOfChannels(), NULL )
, m_uiBuffersize( uiBuffersize )
, m_bManagedBuffer( false )
{
	// Parameter prüfen
	if (pdsSource->GetNumberOfChannels() != (unsigned int) vpfBuffer.size())
		ITA_EXCEPT1(INVALID_PARAMETER, "Number of buffers does not meet the datasource's number of channels");

	// Puffer erzeugen
	for (unsigned int i=0; i<pdsSource->GetNumberOfChannels(); i++)
		m_vpfBufferX[i] = (const float*) m_vpfBuffer[i];

	m_uiWriteCursor = 0;
}

ITABufferDatasink::~ITABufferDatasink() {
	if (m_bManagedBuffer)
		std::for_each(m_vpfBuffer.begin(), m_vpfBuffer.end(), fm_free);
}

unsigned int ITABufferDatasink::GetBuffersize() const {
	return m_uiBuffersize;
}

std::vector<float*> ITABufferDatasink::GetBuffers() const {
	return m_vpfBuffer;
}

unsigned int ITABufferDatasink::GetWriteCursor() const {
	return m_uiWriteCursor;
}

void ITABufferDatasink::SetWriteCursor(unsigned int uiWriteCursor) {
	m_uiWriteCursor = (std::min)(uiWriteCursor, m_uiBuffersize);
}

void ITABufferDatasink::Transfer(unsigned int uiSamples) {
	/*
	 *  Hinweis: Auch wenn der Puffer schon vollgeschrieben wurde,
	 *           so werden trotzdem die Daten von der Datenquelle 
	 *           geholt und dann ins Nirvana geworfen...
	 */

	if (m_pdsSource) {
		// Anzahl der zu transferrierenden Blöcke bestimmen
		unsigned int b = m_pdsSource->GetBlocklength();
		unsigned int n = uprdivu(uiSamples, b);

		for (unsigned int i=0; i<n; i++) {
			// Anzahl der zu speichernden Samples bestimmen
			unsigned int m = (std::min)(m_uiBuffersize - m_uiWriteCursor, b);

			for (unsigned int j=0; j<m_pdsSource->GetNumberOfChannels(); j++) {
				const float* pfSrc = m_pdsSource->GetBlockPointer(j, &m_siState);
				if (m > 0) {
					if (pfSrc)
						memcpy(m_vpfBuffer[j] + m_uiWriteCursor, pfSrc, m*sizeof(float));
					else
						memset(m_vpfBuffer[j] + m_uiWriteCursor, 0, m*sizeof(float));
				} 
			}
				
			m_pdsSource->IncrementBlockPointer();
			m_uiWriteCursor += m;

			m_siState.nSamples += b;
			m_siState.dTimecode = (double) (m_siState.nSamples) / m_pdsSource->GetSamplerate();
		}
	}	
}
