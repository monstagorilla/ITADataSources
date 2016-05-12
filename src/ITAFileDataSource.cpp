#include "ITAFileDataSource.h"
#include <ITAAudiofileReader.h>

#ifndef WIN32
#include <memory.h>
#endif

ITAFileDatasource::ITAFileDatasource(std::string sFilename,
							        unsigned int uiBlocklength,
							        bool bLoopMode)
: ITABufferDatasource()
{
	m_sFilename = sFilename;
	m_uiFileCapacity = 0;

	assert(uiBlocklength > 0);

	// Audiodatei vollständig in den Speicher laden
	// TODO: Ausnahme auslösen, falls Spektraldaten!
	ITAAudiofileReader* pReader = ITAAudiofileReader::create(sFilename);
	ITAAudiofileProperties props = pReader->getAudiofileProperties();
	m_uiFileCapacity = props.uiLength;

	if (m_uiFileCapacity > 0) {
		// Länge auf Vielfaches der Blocklänge erweitern
		unsigned int k = (m_uiFileCapacity / uiBlocklength);
		if (m_uiFileCapacity %  uiBlocklength) k++;
		unsigned int uiCapacity = k * uiBlocklength;

		// Datenpuffer allozieren und Daten hinein kopieren
		for (unsigned int i=0; i<props.uiChannels; i++) {
			// TODO: Fehlerbehandlung!
			float* pfData = new float[uiCapacity];

			// Den letzten Block mit Nullen vorinitialisieren, falls Capacity > FileCapacity
			memset(pfData + (k-1)*uiBlocklength, 0, uiBlocklength * sizeof(float));

			vpfData.push_back(pfData);
		}

		pReader->read(props.uiLength, vpfData);

		// Unterliegende Puffer-Datenquelle initialisieren
		Init(vpfData, uiCapacity, props.dSamplerate, uiBlocklength, bLoopMode);
	}

	delete pReader;
}

ITAFileDatasource::~ITAFileDatasource() {
	// Puffer freigeben
	for (unsigned int i=0; i<vpfData.size(); i++) delete[] vpfData[i];
}

unsigned int ITAFileDatasource::GetFileCapacity() const { return m_uiFileCapacity; }
std::string ITAFileDatasource::GetFileName() const { return m_sFilename; }



