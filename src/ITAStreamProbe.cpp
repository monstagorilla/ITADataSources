#include "ITAStreamProbe.h"
#include <ITABufferedAudioFileWriter.h>

#include <cassert>


ITAStreamProbe::ITAStreamProbe( ITADatasource* pDatasource, const std::string& sFilePath /* = "" */, ITAQuantization iQuantization /* = ITAQuantization::ITA_FLOAT */ )
	: m_pDataSource( pDatasource )
	, m_pBufferedWriter( NULL )
{
	assert( pDatasource != NULL );

	m_dSampleRate = pDatasource->GetSampleRate();
	m_iChannels = int( pDatasource->GetNumberOfChannels() );
	m_iBlockLength = int( pDatasource->GetBlocklength() );

	m_sfBuffer.init( m_iChannels, m_iBlockLength, true );
	
	for( int i = 0; i < m_iChannels; i++ )
		m_vbDataPresent.push_back( false );

	m_iRecordedSamples = 0;

	ITAAudiofileProperties props;
	props.iChannels = m_iChannels;
	props.dSampleRate = m_dSampleRate;
	props.eDomain = ITADomain::ITA_TIME_DOMAIN;
	props.eQuantization = iQuantization;

	m_pBufferedWriter = ITABufferedAudiofileWriter::create( props );
	if( !sFilePath.empty() )
		m_pBufferedWriter->SetFilePath( sFilePath );
}

ITAStreamProbe::~ITAStreamProbe()
{
	delete m_pBufferedWriter;
}

std::string ITAStreamProbe::GetFilePath() const
{
	if( m_pBufferedWriter )
		return m_pBufferedWriter->GetFilePath();
	else
		return "";
}

void ITAStreamProbe::SetFilePath( const std::string& sFilePath )
{
	m_pBufferedWriter->SetFilePath( sFilePath );
}

unsigned int ITAStreamProbe::GetBlocklength() const
{
	return ( unsigned int ) ( m_iBlockLength );
}

unsigned int ITAStreamProbe::GetNumberOfChannels() const
{
	return ( unsigned int ) ( m_iChannels );
}

double ITAStreamProbe::GetSampleRate() const
{
	return m_dSampleRate;
}

const float* ITAStreamProbe::GetBlockPointer( unsigned int uiChannel, const ITAStreamInfo* pStreamInfo )
{
	const float* pfData = m_pDataSource->GetBlockPointer( uiChannel, pStreamInfo );

	if( pfData == nullptr )
		return NULL;

	// Only use first block pointer call on this channel
	if( m_vbDataPresent[ uiChannel ] == false )
	{
		m_sfBuffer[ uiChannel ].write( pfData, GetBlocklength() );
		m_vbDataPresent[ uiChannel ] = true;
	}

	return pfData;
}

void ITAStreamProbe::IncrementBlockPointer()
{
	// Moves the internal buffer of a single block into the file writer
	m_pBufferedWriter->write( &m_sfBuffer );

	m_iRecordedSamples = m_iRecordedSamples + GetBlocklength();

	// Data reset indication
	for( int i = 0; i < m_iChannels; i++ )
		m_vbDataPresent[ i ] = false;

	m_sfBuffer.zero(); // jst: necessary?

	m_pDataSource->IncrementBlockPointer();
}
