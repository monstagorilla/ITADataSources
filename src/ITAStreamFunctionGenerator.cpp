#include "ITAStreamFunctionGenerator.h"

#include <ITAConstants.h>
#include <ITADataSourceRealization.h>
#include <ITANumericUtils.h>
#include <ITAFastMath.h>
#include <cmath>
#include <cassert>
#include <string>
#include <cstdlib>

#ifndef WIN32
#include <memory.h>
#endif

ITAStreamFunctionGenerator::ITAStreamFunctionGenerator(unsigned int uiChannels,
	                                                   double dSamplerate,
				                                       unsigned int uiBlocklength,
						                               int iSignalFunction,
						                               double dFrequency,
						                               float fAmplitude,
						                               bool bPeriodic)
: ITADatasourceRealization(uiChannels, dSamplerate, uiBlocklength),

  // Initial values
  m_iFunction(iSignalFunction),
  m_bPeriodic(bPeriodic),
  m_bMuted(false),
  m_fAmplitude(fAmplitude),
  m_iSampleCount(0),
  m_fPhase(0.0f)
{
	assert(uiChannels > 0);
	assert(dSamplerate > 0);
	assert(uiBlocklength > 0);

	SetFrequency(dFrequency);
	Reset();
}

void ITAStreamFunctionGenerator::Reset()
{
	m_iSampleCount = 0;
	m_fPhase = 0.0f;
}

int ITAStreamFunctionGenerator::GetFunction() const
{
	return m_iFunction;
}

void ITAStreamFunctionGenerator::SetFunction(int iFunction)
{
	m_iFunction = iFunction;
}

double ITAStreamFunctionGenerator::GetFrequency() const
{
	return m_dSampleRate / (double) m_iPeriodLengthSamples;
}

void ITAStreamFunctionGenerator::SetFrequency(double dFrequency)
{
	assert( dFrequency >= 0 );
	m_iPeriodLengthSamples = (int) round( m_dSampleRate / dFrequency );
}

int ITAStreamFunctionGenerator::GetPeriodAsSamples() const
{
	return m_iPeriodLengthSamples;
}

void ITAStreamFunctionGenerator::SetPeriodAsSamples(int iNumSamples)
{
	assert( iNumSamples >= 0 );
	m_iPeriodLengthSamples = iNumSamples;
}

double ITAStreamFunctionGenerator::GetPeriodAsTime() const
{
	return (double) m_iPeriodLengthSamples / m_dSampleRate;
}

void ITAStreamFunctionGenerator::SetPeriodAsTime(double dPeriodLength)
{
	m_iPeriodLengthSamples = (int) round(dPeriodLength * m_dSampleRate);
}

bool ITAStreamFunctionGenerator::IsPeriodic() const
{
	return m_bPeriodic;
}

void ITAStreamFunctionGenerator::SetPeriodic(bool bPeriodic)
{
	m_bPeriodic = bPeriodic;
}

bool ITAStreamFunctionGenerator::IsMuted() const
{
	return m_bMuted;
}

void ITAStreamFunctionGenerator::SetMuted(bool bMuted)
{
	m_bMuted = bMuted;
}

float ITAStreamFunctionGenerator::GetAmplitude() const
{
	return m_fAmplitude;
}

void ITAStreamFunctionGenerator::SetAmplitude(float fAmplitude) 
{
	m_fAmplitude = (float) fAmplitude;
}

void ITAStreamFunctionGenerator::ProcessStream(const ITAStreamInfo* pStreamInfo) 
{
	// Generate the next output samples
	float* pfOutputData = GetWritePointer(0);
	fm_zero(pfOutputData, m_uiBlocklength);

	// Variables
	int N = m_iPeriodLengthSamples;
	float a = m_fAmplitude;
	
	float omega;
	float gradient = a / (float) (N-1);	// Steigung der Sägezahn und Dreieck
	int iZero;							// Offset Übergang 1=>0 bei Rechteck
	int iNumSamples;					// Anzahl zu erzeugender Samples
	
	switch (m_iFunction) {

	case SINE:

		omega = ITAConstants::TWO_PI_F / (float) N; // 2*pi/T
		
		iNumSamples = m_bPeriodic ? (int) m_uiBlocklength : (std::min)( (int) m_uiBlocklength, m_iPeriodLengthSamples - m_iSampleCount);

		for (int i=0;i<iNumSamples;i++) {
			pfOutputData[i] = a * sin(omega*i + m_fPhase);
		}

		m_fPhase = fmodf( iNumSamples*omega + m_fPhase, ITAConstants::TWO_PI_F );

		m_iSampleCount += iNumSamples;
		
		break;

	case TRIANGLE:

		iNumSamples = (m_bPeriodic ? (int)m_uiBlocklength : (std::min)( (int)m_uiBlocklength, m_iPeriodLengthSamples - m_iSampleCount) );

		for (int i=0;i<iNumSamples;i++) {
			float x =  fmodf((float) m_iSampleCount++, (float) m_iPeriodLengthSamples);

			pfOutputData[i] = x < N/2 ?  2*(x*gradient) : 2*((-x*gradient)+a);
		}

		break;
		
	case SAWTOOTH:  // Sägezahn

		iNumSamples = (m_bPeriodic ? (int) m_uiBlocklength : (std::min)( (int) m_uiBlocklength, m_iPeriodLengthSamples - m_iSampleCount) );
		
		for (int i=0;i<iNumSamples;i++) {

			float x = fmodf((float) m_iSampleCount++, (float) m_iPeriodLengthSamples);
			pfOutputData[i] = a * (x*gradient);
		}

		break;
			
	case RECTANGLE:

			// Position des Wechsels von 1 zu 0 innerhalb einer Periode
			iZero = (int)(m_iPeriodLengthSamples / 2);
			iNumSamples = (m_bPeriodic ? (int) m_uiBlocklength : (std::min)((int) m_uiBlocklength, m_iPeriodLengthSamples - m_iSampleCount) );

			for (int i=0; i<iNumSamples; i++) 
			{
				// Position innerhalb einer Periodenlänge
				float x = fmodf((float) m_iSampleCount++, (float) m_iPeriodLengthSamples);
				int iOffset = (int) roundf(x);
				pfOutputData[i] = (iOffset < iZero ? a : 0);
			}

			break;

	case DIRAC:
		
		iNumSamples = (m_bPeriodic ? (int) m_uiBlocklength : (std::min)( (int) m_uiBlocklength, m_iPeriodLengthSamples - m_iSampleCount ) );

		if (m_bPeriodic) {
			
			for (int i=0;i<iNumSamples;i++) pfOutputData[i] = ( m_iSampleCount++ % N ? 0 : a );
		} else {
			pfOutputData[0] = (m_iSampleCount == 0 ? a : 0);
			for (int i=1;i<iNumSamples;i++) pfOutputData[i] = 0;
			m_iSampleCount += iNumSamples;
		}

		break;

	case WHITE_NOISE: 

		srand(100);
		for(unsigned int i= 0; i< m_uiBlocklength; i++)
		pfOutputData[i] = a * (float)rand()/ RAND_MAX;
		
		break;	
	// ...

	default:
		// Case: Invalid signal function => Generate zeros
		for (unsigned int i=0; i<m_uiBlocklength; i++)
			pfOutputData[i] = 0;
	}

	/*
	 *  Since all output channels share the same data,
	 *  we calculate it just for the first channel and
	 *  then copy the data into all further channels.
	 */

	for (unsigned int c=1; c<m_uiChannels; c++)
		memcpy( GetWritePointer(c), pfOutputData, m_uiBlocklength * sizeof(float) );		
	
	IncrementWritePointer();
}

