#include "ITAStreamAmplifier.h"

#include <ITAException.h>
#include <ITADataSourceRealization.h>

#ifndef WIN32
#include <memory.h>
#endif

ITAStreamAmplifier::ITAStreamAmplifier(ITADatasource* pdsInput, float fInitialGain)
: ITADatasourceRealization(pdsInput->GetNumberOfChannels(),
                           pdsInput->GetSampleRate(),
						   pdsInput->GetBlocklength()),
  m_pInputDatasource(pdsInput),
  m_bMuted(false),
  m_fCurGain(fInitialGain),
  m_fPrevGain(0)
{ }

ITAStreamAmplifier::~ITAStreamAmplifier() { }

ITADatasource* ITAStreamAmplifier::GetInputDatasource() const {
	return m_pInputDatasource;
}

bool ITAStreamAmplifier::IsMuted() const {
	return m_bMuted;	
}

void ITAStreamAmplifier::SetMuted(bool bMuted) {
	m_bMuted = bMuted;
}

float ITAStreamAmplifier::GetGain() const {
	return m_fCurGain;	
}

void ITAStreamAmplifier::SetGain(float fGain) {
	/* [fwe] For Ambisonics we also allow negative gains 
	if (fGain < 0)
		ITA_EXCEPT1(INVALID_PARAMETER, "Gain must not be negative");
	*/

	m_fCurGain = fGain;
}

void ITAStreamAmplifier::ProcessStream(const ITAStreamInfo* pStreamInfo) {
	bool bMuted = m_bMuted;
	float fGain = m_fCurGain;

	for (unsigned int i=0; i<m_uiChannels; i++) {
		const float* pfInputData = m_pInputDatasource->GetBlockPointer(i, pStreamInfo);
		if (pfInputData == NULL) bMuted = true;
		float* pfOutputData = GetWritePointer(i);

		if (bMuted || !pfInputData)
			// Stille!
			memset(pfOutputData, 0, m_uiBlocklength*sizeof(float));
		else
			if (fGain == m_fPrevGain) {
				for (unsigned int j=0; j<m_uiBlocklength; j++)
					pfOutputData[j] = pfInputData[j] * fGain;
			} else {
				// Lineare Interpolation für weichen Übergang
				float s = (fGain - m_fPrevGain) / (float) m_uiBlocklength;
				for (unsigned int j=0; j<m_uiBlocklength; j++)
					pfOutputData[j] = pfInputData[j] * (m_fPrevGain + s*j);
			}
	}

	m_fPrevGain = fGain;

	IncrementWritePointer();
}

void ITAStreamAmplifier::PostIncrementBlockPointer() {
	m_pInputDatasource->IncrementBlockPointer();
}
