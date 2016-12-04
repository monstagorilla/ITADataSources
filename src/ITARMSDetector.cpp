#include "ITARMSDetector.h"

#include <ITAException.h>
#include <ITANumericUtils.h>

#include <cmath>

ITARMSDetector::ITARMSDetector( ITADatasource* pDataSource )
	: m_pDataSource( pDataSource )
{
	m_dSamplerate = pDataSource->GetSampleRate();
	m_uiChannels = pDataSource->GetNumberOfChannels();
	m_uiBlocklength = pDataSource->GetBlocklength();
	m_pfRMSs = 0;

	if( ( m_uiBlocklength == 0 ) || ( m_uiChannels == 0 ) || ( m_dSamplerate == 0 ) )
		ITA_EXCEPT0( INVALID_PARAMETER );

	m_pfRMSs = new float[ m_uiChannels ];

	Reset();
}

ITARMSDetector::~ITARMSDetector()
{
	delete[] m_pfRMSs;
};

void ITARMSDetector::Reset()
{
	m_cs.enter();
	for( unsigned int c = 0; c < m_uiChannels; c++ )
		m_pfRMSs[ c ] = 0.00001f;

	m_fOverallRMS = 0;
	m_uiOverallRMSChannel = 0;
	m_cs.leave();
}

void ITARMSDetector::GetOverallRMS( float* pfPeak, unsigned int* puiChannel, bool bReset )
{
	m_cs.enter();

	if( pfPeak )
		*pfPeak = m_fOverallRMS;

	if( puiChannel )
		*puiChannel = m_uiOverallRMSChannel;

	if( bReset )
	{
		m_fOverallRMS = 0;
		m_uiOverallRMSChannel = 0;
	}

	m_cs.leave();
}

void ITARMSDetector::GetOverallRMSDecibel( double* pdPeakDecibel, unsigned int* puiChannel, bool bReset )
{
	m_cs.enter();

	if( pdPeakDecibel )
		*pdPeakDecibel = ratio_to_db20( m_fOverallRMS );

	if( puiChannel )
		*puiChannel = m_uiOverallRMSChannel;

	if( bReset )
	{
		m_fOverallRMS = 0;
		m_uiOverallRMSChannel = 0;
	}

	m_cs.leave();
}

float ITARMSDetector::GetRMS( unsigned int uiChannel, bool bReset )
{
	if( uiChannel >= m_uiChannels )
		ITA_EXCEPT0( INVALID_PARAMETER );

	m_cs.enter();

	float fResult = m_pfRMSs[ uiChannel ];
	if( bReset )
		m_pfRMSs[ uiChannel ] = 0;

	m_cs.leave();

	return fResult;
}

double ITARMSDetector::GetRMSDecibel( unsigned int uiChannel, bool bReset )
{
	return ratio_to_db20( GetRMS( uiChannel, bReset ) );
}

void ITARMSDetector::GetRMSs( float* pfDest, bool bReset )
{
	if( !pfDest )
		ITA_EXCEPT0( INVALID_PARAMETER );

	m_cs.enter();
	for( unsigned int c = 0; c < m_uiChannels; c++ )
	{
		pfDest[ c ] = m_pfRMSs[ c ];
		if( bReset )
			m_pfRMSs[ c ] = 0;
	}
	m_cs.leave();
}

void ITARMSDetector::GetRMSs( std::vector<float>& vfDest, bool bReset )
{
	if( ( ( unsigned int ) vfDest.size() ) < m_uiChannels )
		vfDest.resize( m_uiChannels );

	m_cs.enter();
	for( unsigned int c = 0; c < m_uiChannels; c++ )
	{
		vfDest[ c ] = m_pfRMSs[ c ];
		if( bReset ) 
			m_pfRMSs[ c ] = 0;
	}
	m_cs.leave();
}

void ITARMSDetector::GetRMSsDecibel( std::vector<double>& vdDestDecibel, bool bReset )
{
	if( ( ( unsigned int ) vdDestDecibel.size() ) < m_uiChannels )
		vdDestDecibel.resize( m_uiChannels );

	m_cs.enter();
	for( unsigned int c = 0; c < m_uiChannels; c++ )
	{
		vdDestDecibel[ c ] = ratio_to_db20( m_pfRMSs[ c ] );
		if( bReset )
			m_pfRMSs[ c ] = 0;
	}
	m_cs.leave();
}

const float* ITARMSDetector::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
{
	const float* pfData = m_pDataSource->GetBlockPointer( uiChannel, pStreamInfo );

	if( pfData )
	{
		// TODO: Ist es wirklich nötig bei jedem GBP die CS zu betreten? :-(
		m_cs.enter();

		// Daten analysieren
		for( unsigned int i = 0; i < m_uiBlocklength; i++ )
		{
			float fAbs = std::abs( pfData[ i ] );

			if( fAbs > m_pfRMSs[ uiChannel ] )
				m_pfRMSs[ uiChannel ] = fAbs;

			if( fAbs > m_fOverallRMS )
			{
				m_fOverallRMS = fAbs;
				m_uiOverallRMSChannel = uiChannel;
			}
		}

		m_cs.leave();
	}

	return pfData;
}

void ITARMSDetector::IncrementBlockPointer()
{
	m_pDataSource->IncrementBlockPointer();
}
