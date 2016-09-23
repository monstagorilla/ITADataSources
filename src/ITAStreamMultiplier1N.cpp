#include "ITAStreamMultiplier1N.h"

#include <ITAException.h>
#include <ITADataSourceRealization.h>

#include <cassert>
#include <iostream>
#include <algorithm>

#ifndef WIN32
#include <memory.h>
#endif

using namespace std;

ITAStreamMultiplier1N::ITAStreamMultiplier1N(ITADatasource* pdsInput,int iOutputChannels, unsigned int uiFadeLength)
: ITADatasourceRealization(iOutputChannels, pdsInput->GetSampleRate(), pdsInput->GetBlocklength()),
  m_pInputDatasource(pdsInput),
  m_bMuted(false),
  m_iOutputChannels(iOutputChannels),
  m_vfCurrentGains(iOutputChannels, 1.0F),
  m_vfNewGains(iOutputChannels,1.0F)
{
	// Security check: Always mono input
	assert( pdsInput->GetNumberOfChannels() == 1 );
	// At least 1 output channel
	assert ( iOutputChannels >= 1 );

	SetFadeLength(uiFadeLength);
}

ITAStreamMultiplier1N::~ITAStreamMultiplier1N() { }

ITADatasource* ITAStreamMultiplier1N::GetInputDatasource() const {
	return m_pInputDatasource;
}

unsigned int ITAStreamMultiplier1N::GetFadeLength() const {
	m_cs.enter();
	unsigned int uiResult = m_uiFadeLength;
	m_cs.leave();
	return uiResult;
}

void ITAStreamMultiplier1N::SetFadeLength(unsigned int uiFadeLength) {
	m_cs.enter();
	m_uiFadeLength = (std::min)(uiFadeLength, m_uiBlocklength);
	m_cs.leave();
}

bool ITAStreamMultiplier1N::IsMuted() const {
	m_cs.enter();
	bool bResult = m_bMuted;
	m_cs.leave();
	return bResult;	
}

void ITAStreamMultiplier1N::SetMuted(bool bMuted) {
	m_cs.enter();
	m_bMuted = bMuted;
	m_cs.leave();
}

// GetGain function for a desired channel
float ITAStreamMultiplier1N::GetGain(unsigned int uiChannel) const
{
	if(uiChannel >= m_iOutputChannels)
	{
		ITA_EXCEPT1(INVALID_PARAMETER,"Invalid channel number has been entered");	
	}
    ITACriticalSectionLock lk(m_cs);
	return (m_vfCurrentGains[uiChannel] );
}

// SetGain function for a desired channel

void ITAStreamMultiplier1N::SetGain(unsigned int uiChannel, float fGain)
{
	if((uiChannel < 0)|| (uiChannel >= m_iOutputChannels))
	{
		ITA_EXCEPT1(INVALID_PARAMETER,"Invalid channel number has been entered");	
	}

	/* [fwe] For Ambisonics we also allow negative gains 
	if( fGain < 0)
	{
		ITA_EXCEPT1(INVALID_PARAMETER,"Gain cannot be negative");	
	}
	*/

	m_cs.enter();
	m_vfNewGains[uiChannel] = fGain; 
	m_cs.leave();
}

void ITAStreamMultiplier1N::GetGains(std::vector<float>& vfGains) const 
{
	m_cs.enter();
	vfGains = m_vfCurrentGains;
	m_cs.leave();
}

//  The SetGains function accepts a float vector of gains entered by the user and assigns it to 
//  the corresponding gain vector of the class. 
     
	  
void ITAStreamMultiplier1N::SetGains(const std::vector<float>& vfGains) 
{
	if ((int) vfGains.size() != m_iOutputChannels)
		ITA_EXCEPT1(INVALID_PARAMETER, "Wrong number of gains values passed");

	/* [fwe] For Ambisonics we also allow negative gains
	for (unsigned int i=0; i<m_iOutputChannels; i++) 
	{
		if (vfGains[i] < 0) 
		{
			ITA_EXCEPT1(INVALID_PARAMETER, "Gain values must not be negative");
		}
	}
	*/
	
	m_cs.enter();
	m_vfNewGains = vfGains;
	m_cs.leave();
}

void ITAStreamMultiplier1N::ProcessStream(const ITAStreamInfo* pStreamInfo) {
	bool bMuted;
		
	m_cs.enter();
	bMuted = m_bMuted;

	const float* pfInputData = m_pInputDatasource->GetBlockPointer(0, pStreamInfo);

	for (unsigned int i=0; i<m_uiChannels; i++) 
	{
		float* pfOutputData = GetWritePointer(i);	
		
		if (bMuted || !pfInputData) {
			memset(pfOutputData, 0, m_uiBlocklength*sizeof(float));
		} else  {
			float fScale = (m_vfNewGains[i] - m_vfCurrentGains[i]) / (float) (m_uiFadeLength-1);

			for (unsigned int j=0; j<m_uiFadeLength; j++)
				pfOutputData[j] = pfInputData[j] * (m_vfCurrentGains[i] + j*fScale);

			for (unsigned int j=m_uiFadeLength; j<m_uiBlocklength; j++)
				pfOutputData[j] = pfInputData[j] * m_vfNewGains[i];
		}

		m_vfCurrentGains[i] = m_vfNewGains[i];
	}
	
	m_cs.leave();

	IncrementWritePointer();
}

void ITAStreamMultiplier1N::PostIncrementBlockPointer() {
	if (m_pInputDatasource) m_pInputDatasource->IncrementBlockPointer();
}
