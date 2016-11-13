#include "ITAPeakDetector.h"

#include <ITAException.h>
#include <ITANumericUtils.h>

#include <cmath>

ITAPeakDetector::ITAPeakDetector( ITADatasource* pDataSource )
	: m_pDataSource( pDataSource )
{
	m_dSamplerate = pDataSource->GetSampleRate();
	m_uiChannels = pDataSource->GetNumberOfChannels();
	m_uiBlocklength = pDataSource->GetBlocklength();
	m_pfPeaks = 0;

	// Unspezifizierte Parameter werden nicht erlaubt!
	if ((m_uiBlocklength == 0) ||
        (m_uiChannels == 0) ||
		(m_dSamplerate == 0)) ITA_EXCEPT0(INVALID_PARAMETER);

	// Analyse-Resourcen anlegen
	m_pfPeaks = new float[m_uiChannels];

	Reset();
}

ITAPeakDetector::~ITAPeakDetector() {
	// Analyse-Resourcen freigeben
	delete[] m_pfPeaks;
};

void ITAPeakDetector::Reset() {
	m_cs.enter();
	for (unsigned int c=0; c<m_uiChannels; c++) m_pfPeaks[c] = 0.00001f;
	m_fOverallPeak = 0;
	m_uiOverallPeakChannel = 0;
	m_cs.leave();
}

void ITAPeakDetector::GetOverallPeak(float* pfPeak, unsigned int* puiChannel, bool bReset) {
	m_cs.enter();

	if (pfPeak) *pfPeak = m_fOverallPeak;
	if (puiChannel) *puiChannel = m_uiOverallPeakChannel;

	if (bReset) {
		m_fOverallPeak = 0;
		m_uiOverallPeakChannel = 0;	
	}

	m_cs.leave();
}

void ITAPeakDetector::GetOverallPeakDecibel(double* pdPeakDecibel, unsigned int* puiChannel, bool bReset) {
	m_cs.enter();

	if (pdPeakDecibel) *pdPeakDecibel = ratio_to_db20(m_fOverallPeak);
	if (puiChannel) *puiChannel = m_uiOverallPeakChannel;

	if (bReset) {
		m_fOverallPeak = 0;
		m_uiOverallPeakChannel = 0;	
	}

	m_cs.leave();
}

float ITAPeakDetector::GetPeak(unsigned int uiChannel, bool bReset) {
	// Bereichsprüfung des Kanalindex
	if (uiChannel >= m_uiChannels) ITA_EXCEPT0(INVALID_PARAMETER);

	m_cs.enter();

	float fResult = m_pfPeaks[uiChannel];
	if (bReset) m_pfPeaks[uiChannel] = 0;

	m_cs.leave();

	return fResult;
}

double ITAPeakDetector::GetPeakDecibel(unsigned int uiChannel, bool bReset) {
	return ratio_to_db20(GetPeak(uiChannel));
}

void ITAPeakDetector::GetPeaks(float* pfDest, bool bReset) {
	// Nullzeiger abfangen
	if (!pfDest) ITA_EXCEPT0(INVALID_PARAMETER);

	m_cs.enter();
	for (unsigned int c=0; c<m_uiChannels; c++) {
		pfDest[c] = m_pfPeaks[c];
		if (bReset) m_pfPeaks[c] = 0;
	}
	m_cs.leave();
}

void ITAPeakDetector::GetPeaks(std::vector<float>& vfDest, bool bReset) {
	// Vektor konditionieren
	if (((unsigned int) vfDest.size()) < m_uiChannels) vfDest.resize(m_uiChannels);

	m_cs.enter();
	for (unsigned int c=0; c<m_uiChannels; c++) {
		vfDest[c] = m_pfPeaks[c];
		if (bReset) m_pfPeaks[c] = 0;
	}
	m_cs.leave();
}

void ITAPeakDetector::GetPeaksDecibel(double* pdDestDecibel, bool bReset) {
	// Nullzeiger abfangen
	if (!pdDestDecibel) ITA_EXCEPT0(INVALID_PARAMETER);

	m_cs.enter();
	for (unsigned int c=0; c<m_uiChannels; c++) {
		pdDestDecibel[c] = ratio_to_db20(m_pfPeaks[c]);
		if (bReset) m_pfPeaks[c] = 0;
	}
	m_cs.leave();
}

void ITAPeakDetector::GetPeaksDecibel(std::vector<double>& vdDestDecibel, bool bReset) {
	// Vektor konditionieren
	if (((unsigned int) vdDestDecibel.size()) < m_uiChannels) vdDestDecibel.resize(m_uiChannels);

	m_cs.enter();
	for (unsigned int c=0; c<m_uiChannels; c++) {
		vdDestDecibel[c] = ratio_to_db20(m_pfPeaks[c]);
		if (bReset) m_pfPeaks[c] = 0;
	}
	m_cs.leave();
}

const float* ITAPeakDetector::GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo) {
	// Datenzeiger seinerseits bei der angeschlossenen Datenquelle abrufen
	const float* pfData = m_pDataSource->GetBlockPointer(uiChannel, pStreamInfo);

	if (pfData) {
		// TODO: Ist es wirklich nötig bei jedem GBP die CS zu betreten? :-(
		m_cs.enter();

		// Daten analysieren
		for (unsigned int i=0; i<m_uiBlocklength; i++) {
			float fAbs = std::abs(pfData[i]);

			if (fAbs > m_pfPeaks[uiChannel]) m_pfPeaks[uiChannel] = fAbs;

			if (fAbs > m_fOverallPeak) {
				m_fOverallPeak = fAbs;
				m_uiOverallPeakChannel = uiChannel;
			}
		}

		m_cs.leave();
	}

	// Daten "weitergeben"
	return pfData;
}

void ITAPeakDetector::IncrementBlockPointer() {
	// Blockzeiger der angeschlossenen Datenquelle inkrementieren
	m_pDataSource->IncrementBlockPointer();
}
