#include "ITAStreamDetector.h"

#include <ITAConstants.h>
#include <ITAException.h>
#include <ITANumericUtils.h>

#include <cmath>

ITAStreamDetector::ITAStreamDetector( ITADatasource* pDataSource, const int iMode )
	: m_pDataSource( pDataSource )
	, m_iMode( iMode )
	, m_bProfilerEnabled( false )
{
	m_dSamplerate = pDataSource->GetSampleRate();
	m_iChannels = int( pDataSource->GetNumberOfChannels() );
	m_iBlocklength = int( pDataSource->GetBlocklength() );

	// Unspezifizierte Parameter werden nicht erlaubt!
	if( ( m_iBlocklength == 0 ) || ( m_iChannels == 0 ) || ( m_dSamplerate == 0 ) )
		ITA_EXCEPT1( INVALID_PARAMETER, "Could not create stream detector" );

	m_vfPeaks.resize( m_iChannels, 0.0f );
	m_vdRMSSquaredSums.resize( m_iChannels, 0.0f );

	Reset();
}

void ITAStreamDetector::Reset()
{
	m_cs.enter();

	for( size_t c = 0; c < m_vfPeaks.size(); c++ )
	{
		m_vfPeaks[ c ] = 0.0f;
		m_vdRMSSquaredSums[ c ] = 0.0f;
	}

	m_fOverallPeak = 0;
	m_iOverallPeakChannel = 0;
	m_iRMSBlocks = 0;

	m_cs.leave();
}

int ITAStreamDetector::GetMode() const
{
	return m_iMode;
}

void ITAStreamDetector::SetMode( const int iMode )
{
	if( iMode < ITAStreamDetector::DEACTIVATED || iMode > ITAStreamDetector::RMS )
		ITA_EXCEPT1( INVALID_PARAMETER, "Unkown mode for a stream detector" );
	m_iMode = iMode;
}

float ITAStreamDetector::GetOverallPeak( int& iChannel, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::PEAK && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	m_cs.enter();

	iChannel = m_iOverallPeakChannel;
	const float fOverallPeak = m_fOverallPeak;

	if( bReset )
	{
		m_fOverallPeak = 0;
		m_iOverallPeakChannel = 0;
	}

	m_cs.leave();

	return fOverallPeak;
}

double ITAStreamDetector::GetOverallPeakDecibel( int& iChannel, const bool bReset )
{
	return ratio_to_db20( GetOverallPeak( iChannel, bReset ) );
}

float ITAStreamDetector::GetPeak( const int iChannel, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::PEAK && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	if( iChannel >= m_iChannels )
		ITA_EXCEPT1( INVALID_PARAMETER, "Invalid channel number, can not get peak" );

	m_cs.enter();

	const float fResult = m_vfPeaks[ iChannel ];
	if( bReset )
	{
		m_vfPeaks[ iChannel ] = 0.0f;
	}

	m_cs.leave();

	return fResult;
}

double ITAStreamDetector::GetPeakDecibel( int iChannel, const bool bReset )
{
	return ratio_to_db20( GetPeak( iChannel, bReset ) );
}

void ITAStreamDetector::GetPeaks( std::vector< float >& vfDest, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::PEAK && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	m_cs.enter();
	vfDest = m_vfPeaks;
	if( bReset )
	{
		m_fOverallPeak = 0.0f;
		for( size_t c = 0; c < m_iChannels; c++ )
			m_vfPeaks[ c ] = 0.0f;
	}
	m_cs.leave();
}

void ITAStreamDetector::GetPeaksDecibel( std::vector< double >& vdDestDecibel, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::PEAK && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	if( vdDestDecibel.size() != m_vfPeaks.size() )
		vdDestDecibel.resize( m_vfPeaks.size() );

	m_cs.enter();
	for( size_t c = 0; c < m_vfPeaks.size(); c++ )
	{
		vdDestDecibel[ c ] = ratio_to_db20( m_vfPeaks[ c ] );
		if( bReset )
			m_vfPeaks[ c ] = 0.0f;
	}
	m_cs.leave();
}

float ITAStreamDetector::GetOverallRMS( const bool bReset )
{
	if( m_iMode != ITAStreamDetector::RMS && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	m_cs.enter();

	if( m_iRMSBlocks == 0 )
	{
		m_cs.leave();
		return 0.0f;
	}

	double dOverallRMSSums = 0;
	for( size_t i = 0; i < m_vdRMSSquaredSums.size(); i++ )
		dOverallRMSSums += m_vdRMSSquaredSums[ i ] / double( m_iRMSBlocks * m_iBlocklength );

	const float fOverallRMS = float( std::sqrt( dOverallRMSSums / double( m_iChannels ) ) ); // RMS over all channels

	if( bReset )
	{
		m_iRMSBlocks = 0;
		m_vdRMSSquaredSums.resize( m_iChannels, 0.0f );
	}

	m_cs.leave();

	return fOverallRMS;
}

double ITAStreamDetector::GetOverallRMSDecibel( const bool bReset )
{
	return ratio_to_db20( GetOverallRMS( bReset ) );
}

float ITAStreamDetector::GetRMS( const int iChannel, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::RMS && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	if( iChannel >= m_iChannels )
		ITA_EXCEPT1( INVALID_PARAMETER, "Invalid channel number, can not get peak" );

	m_cs.enter();

	const float fResult = std::sqrt( float( m_vdRMSSquaredSums[ iChannel ] / double( m_iRMSBlocks *m_iBlocklength ) ) );
	if( bReset )
		m_vdRMSSquaredSums[ iChannel ] = 0.0f;

	m_cs.leave();

	return fResult;
}

double ITAStreamDetector::GetRMSDecibel( int iChannel, const bool bReset )
{
	return ratio_to_db20( GetRMS( iChannel, bReset ) );
}

void ITAStreamDetector::GetRMSs( std::vector< float >& vfDest, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::RMS && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	m_cs.enter();

	vfDest.resize( m_vdRMSSquaredSums.size(), 0.0f );
	if( m_iRMSBlocks == 0 )
	{
		m_cs.leave();
		return;
	}

	for( size_t c = 0; c < m_vdRMSSquaredSums.size(); c++ )
		vfDest[ c ] = float( std::sqrt( ( m_vdRMSSquaredSums[ c ] / double( m_iRMSBlocks ) ) ) ); // RMS

	if( bReset )
		m_vdRMSSquaredSums.resize( m_iChannels, 0.0f );

	m_cs.leave();
}

void ITAStreamDetector::GetRMSsDecibel( std::vector< float >& vdDestDecibel, const bool bReset )
{
	if( m_iMode != ITAStreamDetector::RMS && m_iMode != ITAStreamDetector::PEAK_AND_RMS )
		ITA_EXCEPT1( MODAL_EXCEPTION, "Can not provide stream detector data because it is not calculated in this mode" );

	m_cs.enter();

	vdDestDecibel.resize( m_vdRMSSquaredSums.size(), ITAConstants::MINUS_INFINITY_F );
	if( m_iRMSBlocks == 0 )
	{
		m_cs.leave();
		return;
	}

	for( size_t c = 0; c < m_vdRMSSquaredSums.size(); c++ )
		vdDestDecibel[ c ] = float( ratio_to_db20( std::sqrt( m_vdRMSSquaredSums[ c ] / double( m_iRMSBlocks * m_iBlocklength ) ) ) ); // RMS

	if( bReset )
		m_vdRMSSquaredSums.resize( m_iChannels, 0.0f );

	m_cs.leave();
}

const float* ITAStreamDetector::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
{
	const float* pfData = m_pDataSource->GetBlockPointer( uiChannel, pStreamInfo );

	const bool bProfilerEnabled = GetProfilerEnabled();
	if( bProfilerEnabled )
		m_sw.start();

	if( pfData && m_iMode != ITAStreamDetector::DEACTIVATED )
	{
		m_cs.enter();

		if( m_iMode == ITAStreamDetector::PEAK || m_iMode == ITAStreamDetector::PEAK_AND_RMS )
		{
			for( int i = 0; i < m_iBlocklength; i++ )
			{
				const float fAbs = std::abs( pfData[ i ] );

				if( fAbs > m_vfPeaks[ uiChannel ] )
					m_vfPeaks[ uiChannel ] = fAbs;

				if( fAbs > m_fOverallPeak )
				{
					m_fOverallPeak = fAbs;
					m_iOverallPeakChannel = uiChannel;
				}
			}
		}

		if( m_iMode == ITAStreamDetector::RMS || m_iMode == ITAStreamDetector::PEAK_AND_RMS )
		{
			for( int i = 0; i < m_iBlocklength; i++ )
			{
				const float fSampleValue = std::abs( pfData[ i ] );
				m_vdRMSSquaredSums[ uiChannel ] += fSampleValue * fSampleValue; // square & sum
			}
		}

		m_cs.leave();
	}

	if( bProfilerEnabled )
		m_sw.stop();

	return pfData;
}

void ITAStreamDetector::IncrementBlockPointer()
{
	m_iRMSBlocks++;
	m_pDataSource->IncrementBlockPointer();
}

void ITAStreamDetector::SetProfilerEnabled( bool bEnabled )
{
	m_bProfilerEnabled = bEnabled;
}

bool ITAStreamDetector::GetProfilerEnabled() const
{
	return m_bProfilerEnabled;
}

double ITAStreamDetector::GetProfilerMeanCalculationTime( bool bReset )
{
	double dResult = 0.0f;

	if( GetProfilerEnabled() )
	{
		dResult = m_sw.mean();
		if( bReset )
			m_sw.reset();
	}

	return dResult;
}

std::string ITAStreamDetector::GetProfilerResult( bool bReset )
{
	std::string sResult;

	if( GetProfilerEnabled() )
	{
		sResult = m_sw.ToString();
		if( bReset )
			m_sw.reset();
	}

	return sResult;
}
