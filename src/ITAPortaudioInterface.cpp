#include <ITAPortaudioInterface.h>

#include <math.h>
#include <portaudio.h>
#include <vector>

#include <ITADataSource.h>
#include <ITADataSourceRealization.h>
#include <ITAStreamInfo.h>

class PaStreamCallbackTimeInfo;

// Portaudio Callback Function
static int PortaudioCallbackFunction( const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* );

class ITAPortaudioSource : public ITADatasourceRealization
{
public:
	inline ITAPortaudioSource( int iChannels, double dSampleRate, int iBufferSize )
		: ITADatasourceRealization( ( unsigned int ) iChannels, dSampleRate, ( unsigned int ) iBufferSize, 16 ) {};
};


ITAPortaudioInterface::ITAPortaudioInterface( double dSampleRate, int iBufferSize )
	: m_vpPaStream( NULL )
{
	m_dSampleRate = dSampleRate;
	m_iBufferSize = iBufferSize; // Darf 0 sein

	if( m_iBufferSize == 0 )
		m_iBufferSize = GetPreferredBufferSize();

	m_iNumInputChannels = -1;
	m_iNumOutputChannels = -1;
	m_sConfigFile.clear();

	m_bInitialized = false;
	m_bOpen = false;
	m_bStreaming = false;

	m_bRecord = false;
	m_bPlayback = false;

	m_iError = ITA_PA_NO_ERROR;
}

ITAPortaudioInterface::~ITAPortaudioInterface()
{
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Initialize( const std::string& )
{
	return ITA_PA_INVALID_DEVICE;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Initialize( int iDriverID )
{
	if( m_bInitialized )
		return ITA_PA_IS_INITIALIZED;

	if( m_bOpen )
		return ITA_PA_IS_OPEN;

	if( m_bStreaming )
		return ITA_PA_IS_STARTED;

	m_iError = ( ITA_PA_ERRORCODE ) Pa_Initialize();

	if( m_iError != ITA_PA_NO_ERROR )
		return m_iError;

	if( iDriverID < 0 || iDriverID >= Pa_GetDeviceCount() )
	{
		Pa_Terminate();
		m_bInitialized = false;
		return ITA_PA_INVALID_DEVICE;
	}

	m_bInitialized = true;

	m_iDriverID = iDriverID;

	m_iNumInputChannels = GetNumInputChannels( m_iDriverID );
	m_iNumOutputChannels = GetNumOutputChannels( m_iDriverID );


	return ITA_PA_NO_ERROR;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Initialize()
{
	if( m_bInitialized )
		return ITA_PA_IS_INITIALIZED;

	if( m_bOpen )
		return ITA_PA_IS_OPEN;

	if( m_bStreaming )
		return ITA_PA_IS_STARTED;

	m_iError = ( ITA_PA_ERRORCODE ) Pa_Initialize();

	if( m_iError != ITA_PA_NO_ERROR ) {
		Pa_Terminate();
		m_bInitialized = false;
		return m_iError;
	}

	m_bInitialized = true;

	m_iDriverID = Pa_GetDefaultOutputDevice();

	m_iNumOutputChannels = GetNumOutputChannels( m_iDriverID );
	m_iNumInputChannels = GetNumInputChannels( m_iDriverID );


	return ITA_PA_NO_ERROR;
}

bool ITAPortaudioInterface::IsPlaybackEnabled() const
{
	return m_bPlayback;
}

void ITAPortaudioInterface::SetPlaybackEnabled( bool bEnabled )
{
	m_bPlayback = bEnabled;
}

bool ITAPortaudioInterface::IsRecordEnabled() const {
	return m_bRecord;
}

void ITAPortaudioInterface::SetRecordEnabled( bool bEnabled )
{
	m_bRecord = bEnabled;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Finalize()
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	if( m_bStreaming )
		return ITA_PA_IS_STARTED;

	m_iError = ( ITA_PA_ERRORCODE ) Pa_Terminate();

	m_iDriverID = -1;

	if( m_iError == ITA_PA_NO_ERROR )
		m_bInitialized = false;

	if( m_oUserData.pdsRecordDatasource != NULL ) {
		delete m_oUserData.pdsRecordDatasource;
		m_oUserData.pdsRecordDatasource = NULL;
	}

	return m_iError;
}


ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Open()
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	if( m_bOpen )
		return ITA_PA_IS_OPEN;

	if( m_oUserData.pdsPlaybackDatasource == NULL && m_bPlayback )
		return ITA_PA_NO_PLAYBACK_DATASOURCE;

	if( m_oUserData.pdsRecordDatasource == NULL && m_bRecord )
		return ITA_PA_NO_RECORD_DATASOURCE;


	if( m_bRecord ) {
		if( ( int ) m_oUserData.pdsRecordDatasource->GetNumberOfChannels() > m_iNumInputChannels )
			return ITA_PA_UNMATCHED_CHANNELS;

		m_oUserData.bRecord = true;
	}

	if( m_bPlayback ) {
		if( ( int ) m_oUserData.pdsPlaybackDatasource->GetNumberOfChannels() > m_iNumOutputChannels )
			return ITA_PA_UNMATCHED_CHANNELS;

		m_oUserData.bPlayback = true;
	}

	PaStream* stream;

	PaStreamParameters inparams;
	inparams.channelCount = GetNumInputChannels( m_iDriverID );
	inparams.device = m_iDriverID;
	inparams.sampleFormat = paFloat32;
	inparams.suggestedLatency = Pa_GetDeviceInfo( inparams.device )->defaultLowInputLatency;
	inparams.hostApiSpecificStreamInfo = NULL;

	PaStreamParameters* pInParams = NULL;
	if( inparams.channelCount > 0 )
		pInParams = &inparams;

	PaStreamParameters outparams;
	outparams.channelCount = GetNumOutputChannels( m_iDriverID );
	outparams.device = m_iDriverID;
	outparams.sampleFormat = paFloat32;
	outparams.suggestedLatency = Pa_GetDeviceInfo( outparams.device )->defaultLowOutputLatency;
	outparams.hostApiSpecificStreamInfo = NULL;

	PaStreamParameters* pOutParams = NULL;
	if( outparams.channelCount > 0 )
		pOutParams = &outparams;

	m_iError = ( ITA_PA_ERRORCODE ) Pa_OpenStream( &stream, pInParams, pOutParams, m_dSampleRate, m_iBufferSize, paNoFlag, PortaudioCallbackFunction, &m_oUserData );

	if( m_iError == ITA_PA_NO_ERROR )
	{
		m_vpPaStream = ( void* ) stream;
		m_bOpen = true;
	}
	else
	{
		m_vpPaStream = NULL;
	}

	return m_iError;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Close()
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	if( !m_bOpen )
		return ITA_PA_IS_NOT_OPEN;

	PaStream* stream = ( PaStream* ) m_vpPaStream;
	m_iError = ( ITA_PA_ERRORCODE ) Pa_CloseStream( stream );

	if( m_iError == ITA_PA_NO_ERROR ) {
		m_vpPaStream = NULL;
		m_bOpen = false;
	}

	return m_iError;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Start()
{
	if( m_bStreaming )
		return ITA_PA_IS_STARTED;

	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	if( !m_bOpen )
		return ITA_PA_IS_NOT_OPEN;

	if( !m_vpPaStream )
		return ITA_PA_INTERNAL_ERROR;

	PaStream* stream = ( PaStream* ) m_vpPaStream;
	m_iError = ( ITA_PA_ERRORCODE ) Pa_StartStream( stream );

	if( m_iError == ITA_PA_NO_ERROR )
		m_bStreaming = true;
	else
		m_bStreaming = false;

	return m_iError;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::Stop()
{
	if( !m_bStreaming )
		return ITA_PA_STREAM_IS_STOPPED;

	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	PaStream* stream = ( PaStream* ) m_vpPaStream;
	m_iError = ( ITA_PA_ERRORCODE ) Pa_StopStream( stream );

	if( m_iError == ITA_PA_NO_ERROR )
		m_bStreaming = false;

	return m_iError;
}

int ITAPortaudioInterface::GetNumDevices() const {
	if( !m_bInitialized )
		return -1;

	return Pa_GetDeviceCount();

}

int ITAPortaudioInterface::GetNumInputChannels( int iDeviceID ) const
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	const PaDeviceInfo* info;
	info = Pa_GetDeviceInfo( iDeviceID );

	return info->maxInputChannels;
}

int ITAPortaudioInterface::GetNumOutputChannels( int iDeviceID ) const
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	const PaDeviceInfo* info;
	info = Pa_GetDeviceInfo( iDeviceID );

	return info->maxOutputChannels;
}

double ITAPortaudioInterface::GetSampleRate() const
{
	return m_dSampleRate;
}

void ITAPortaudioInterface::GetNumChannels( int iDeviceID, int& iNumInputChannels, int& iNumOutputChannels ) const
{
	iNumInputChannels = GetNumInputChannels( iDeviceID );
	iNumOutputChannels = GetNumOutputChannels( iDeviceID );
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::GetDriverSampleRate( int iDeviceID, double& dSampleRate ) const
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	const PaDeviceInfo* info;
	info = Pa_GetDeviceInfo( iDeviceID );

	dSampleRate = info->defaultSampleRate;

	return ITA_PA_NO_ERROR;
}

std::string ITAPortaudioInterface::GetDeviceName( int iDriverID ) const
{
	if( !m_bInitialized )
		return "Portaudio not initialized";

	const PaDeviceInfo* info;
	info = Pa_GetDeviceInfo( iDriverID );

	if( info == NULL )
		return "";

	return info->name;
}

float ITAPortaudioInterface::GetDeviceLatency( int iDriverID ) const
{
	if( !m_bInitialized )
		return -1;

	const PaDeviceInfo* info;
	info = Pa_GetDeviceInfo( iDriverID );

	if( info == NULL )
		return -1;

	return ( float ) info->defaultLowOutputLatency;
}

int ITAPortaudioInterface::GetDefaultInputDevice() const
{
	int iDefaultInputDevice;
	if( !m_bInitialized ) {
		Pa_Initialize();
		iDefaultInputDevice = ( int ) Pa_GetDefaultInputDevice();
		Pa_Terminate();
	}
	else {
		iDefaultInputDevice = ( int ) Pa_GetDefaultInputDevice();
	}

	return iDefaultInputDevice;
}

int ITAPortaudioInterface::GetDefaultOutputDevice() const
{
	int iDefaultOutputDevice;
	if( !m_bInitialized ) {
		Pa_Initialize();
		iDefaultOutputDevice = ( int ) Pa_GetDefaultOutputDevice();
		Pa_Terminate();
	}
	else {
		iDefaultOutputDevice = ( int ) Pa_GetDefaultOutputDevice();
	}

	return iDefaultOutputDevice;
}

ITAPortaudioInterface::ITA_PA_ERRORCODE ITAPortaudioInterface::SetPlaybackDatasource( ITADatasource* pidsDatasource )
{
	if( !m_bInitialized )
		return ITA_PA_NOT_INITIALIZED;

	if( m_bOpen )
		return ITA_PA_IS_OPEN;

	if( int( pidsDatasource->GetBlocklength() ) != m_iBufferSize )
		return ITA_PA_UNMATCHED_BUFFER_SIZE;

	if( pidsDatasource->GetSampleRate() != m_dSampleRate )
		return ITA_PA_UNMATCHED_SAMPLE_RATE;

	m_oUserData.pdsPlaybackDatasource = pidsDatasource;

	m_bPlayback = true;

	return ITA_PA_NO_ERROR;
}

ITADatasource* ITAPortaudioInterface::GetRecordDatasource()
{
	if( !m_bInitialized )
		return NULL;

	if( m_bOpen )
		return NULL;

	if( m_oUserData.pdsRecordDatasource == NULL )
		m_oUserData.pdsRecordDatasource = new ITAPortaudioSource( m_iNumInputChannels, m_dSampleRate, m_iBufferSize );

	m_bRecord = true;

	return m_oUserData.pdsRecordDatasource;
}

void ITAPortaudioInterface::Sleep( float fSeconds ) const
{
	Pa_Sleep( ( long ) fSeconds * 1000 );

	return;
}

std::string ITAPortaudioInterface::GetErrorCodeString( ITA_PA_ERRORCODE err )
{
	if( err == ITA_PA_NOT_INITIALIZED )
		return "Portaudio not initialized";
	if( err == ITA_PA_INVALID_CHANNEL_COUNT )
		return "Portaudio invalid channel count";
	if( err == ITA_PA_UNMATCHED_CHANNELS )
		return "Portaudio channels are not matching";
	if( err == ITA_PA_SAMPLE_FORMAT_NOT_SUPPORTED )
		return "Portaudio sample format not supported";
	if( err == ITA_PA_UNANTICIPATED_HOST_ERROR )
		return "Portaudio unanticipated host error";
	if( err == ITA_PA_INVALID_DEVICE )
		return "Portaudio invalid device";
	if( err == ITA_PA_NO_PLAYBACK_DATASOURCE )
		return "Portaudio no datasource for playback available";
	if( err == ITA_PA_NO_PLAYBACK_DATASOURCE )
		return "Portaudio no datasource for recording available";
	if( err == ITA_PA_DEVICE_UNAVAILABLE )
		return "Portaudio device not available";

	return "Unkown error code";
}

int ITAPortaudioInterface::GetPreferredBufferSize()
{
	return 256;
}

// Portaudio streaming callback function
static int PortaudioCallbackFunction( const void* pInBuffer, void* pOutBuffer, unsigned long ulBuffersize,
	const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* pUserData )
{

	// --= User data =--

	ITAPortaudioInterface::ITAPortaudioUserData* pipudDatasource;
	if( pUserData == NULL )
		return -1;
	else
		pipudDatasource = ( ITAPortaudioInterface::ITAPortaudioUserData* ) pUserData;


	ITAPortaudioSource* pipdsRecordDatasource = ( ITAPortaudioSource* ) pipudDatasource->pdsRecordDatasource;
	ITADatasource* pipdsPlaybackDatasource = pipudDatasource->pdsPlaybackDatasource;
	bool bRecord = pipudDatasource->bRecord;
	bool bPlayback = pipudDatasource->bPlayback;

	ITAStreamInfo oStreamInfo; // DICKES TODO HIER

	// --= Input device =--

	float* in = ( float* ) pInBuffer;

	if( bRecord )
	{
		int iNumInputChannels = pipdsRecordDatasource->GetNumberOfChannels();
		float* pInDataBlock;
		for( unsigned int i = 0; i < ulBuffersize; i++ )
		{
			for( int j = 0; j < iNumInputChannels; j++ )
			{
				int index = i*iNumInputChannels + j;
				pInDataBlock = pipdsRecordDatasource->GetWritePointer( j );
				float fValue = in[ index ];
				if( pInDataBlock )
					pInDataBlock[ i ] = fValue;
			}
		}
		pipdsRecordDatasource->IncrementWritePointer();
	}


	// --= Output device =--

	float* out = ( float* ) pOutBuffer;

	if( bPlayback )
	{
		int iNumOutputChannels = pipdsPlaybackDatasource->GetNumberOfChannels();
		const float* pOutDataBlock;
		std::vector< const float* > vpBlockPointer( iNumOutputChannels );
		for( int j = 0; j < iNumOutputChannels; j++ )
			vpBlockPointer[ j ] = pipdsPlaybackDatasource->GetBlockPointer( j, &oStreamInfo );

		for( unsigned int i = 0; i < ulBuffersize; i++ )
		{
			for( int j = 0; j < iNumOutputChannels; j++ )
			{
				int index = i*iNumOutputChannels + j;
				pOutDataBlock = vpBlockPointer[ j ];
				float fValue = pOutDataBlock ? pOutDataBlock[ i ] : 0.0f;
				out[ index ] = fValue;
			}
		}
		pipdsPlaybackDatasource->IncrementBlockPointer();
	}

	return 0;
}
