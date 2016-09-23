#include "ITAStreamProbe.h"

#include <cassert>
#include <ITABufferedAudioFileWriter.h>

#ifndef WIN32
#include <memory.h>
#endif

ITAStreamProbe::ITAStreamProbe(ITADatasource* pDatasource, const std::string& sFilename, ITAQuantization iQuantization)
{
	assert( pDatasource != NULL );

	m_pDatasource = pDatasource;
	m_sFilename = sFilename;

	m_dSamplerate = pDatasource->GetSampleRate();
	m_uiChannels = pDatasource->GetNumberOfChannels();
	m_uiBlocklength = pDatasource->GetBlocklength();

	ITAAudiofileProperties props;
	props.iChannels = m_uiChannels;
	props.dSampleRate = m_dSamplerate;
	props.eDomain = ITADomain::ITA_TIME_DOMAIN;
	props.eQuantization = iQuantization;

	m_pWriter = ITABufferedAudiofileWriter::create(sFilename, props);

	m_pfBuffer = new float[m_uiChannels * m_uiBlocklength];
	m_pbDataPresent = new bool[m_uiChannels];

	for (unsigned int i=0; i<m_uiChannels; i++) {
		m_pbDataPresent[i] = false;
		m_vpfAlias.push_back(m_pfBuffer + (i*m_uiBlocklength));
	}

	m_iRecordedSamples = 0;
}

ITAStreamProbe::~ITAStreamProbe() {
	delete m_pWriter;
	delete[] m_pfBuffer;
	delete[] m_pbDataPresent;
}

unsigned int ITAStreamProbe::GetBlocklength() const {
	return m_uiBlocklength;
}

unsigned int ITAStreamProbe::GetNumberOfChannels() const {
	return m_uiChannels;
}

double ITAStreamProbe::GetSampleRate() const {
	return m_dSamplerate;
}

const float* ITAStreamProbe::GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo) {
	const float* pfData = m_pDatasource->GetBlockPointer(uiChannel, pStreamInfo);

	// Nur den ersten Block bei Leseanforderung speichern
	if (pfData && !m_pbDataPresent[uiChannel]) {
		memcpy(m_vpfAlias[uiChannel], pfData, m_uiBlocklength * sizeof(float));
		m_pbDataPresent[uiChannel] = true;
	}

	return pfData;
}

void ITAStreamProbe::IncrementBlockPointer() {
	// Gepufferte Daten in die Audiodatei schreiben
	m_pWriter->write(m_uiBlocklength, m_vpfAlias);
	m_iRecordedSamples = m_iRecordedSamples + (int) m_uiBlocklength;

	// Internen Puffer löschen
	for (unsigned int i=0; i<m_uiChannels; i++) m_pbDataPresent[i] = false;
	memset(m_pfBuffer, 0, m_uiChannels * m_uiBlocklength * sizeof(float));

	m_pDatasource->IncrementBlockPointer();
}
