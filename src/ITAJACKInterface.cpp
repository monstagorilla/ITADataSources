#include "ITAJACKInterface.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <signal.h>


#include <ITADatasource.h>
#include <ITADatasourceRealization.h>
#include <ITAStreamInfo.h>


static int ITAJackProcess (jack_nframes_t nframes, void *arg);

jack_client_t *ITAJACKInterface::s_jackClient = NULL;

static void signal_handler(int sig)
{
	jack_client_t *client = ITAJACKInterface::GetJackClient();
	if(client != NULL) {
		fprintf(stderr, "JACKInterface: Closing client ...\n");
		jack_client_close(client);
	}
	fprintf(stderr, "JACKInterface: signal received, exiting ...\n");
	exit(0);
}


class ITAJackSource : public ITADatasourceRealization {
public:
	ITAJackSource(int iChannels, double dSamplerate, int iBufferSize)
		: ITADatasourceRealization((unsigned int) iChannels, dSamplerate, (unsigned int) iBufferSize, 16) {}
};


ITAJACKInterface::ITAJACKInterface()
{
	m_jackClient = NULL;

	m_iNumInputChannels = -1;
	m_iNumOutputChannels = -1;

	m_bInitialized = false;
	m_bOpen = false;
	m_bStreaming = false;

	m_bRecord = false;
	m_bPlayback = false;
	
	m_iError = ITA_JACK_NO_ERROR;
}

ITAJACKInterface::~ITAJACKInterface() { }



ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Initialize(const std::string& clientName) {
	if (m_bInitialized)
		return ITA_JACK_NO_ERROR; // allow multiple Init() calls

	if (m_bOpen)
		return ITA_JACK_IS_OPEN;

	if (m_bStreaming)
		return ITA_JACK_IS_STARTED;

	int i;

	jack_options_t options = JackNullOption; // JackNoStartServer; // 
	jack_status_t status;


	jack_set_error_function(ITAJackMsg);
	jack_set_info_function(ITAJackMsg);

	m_jackClient = jack_client_open (clientName.c_str(), options, &status);

	if (m_jackClient == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
				"status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		return ITA_JACK_OPEN_FAILED;
	}
	
	s_jackClient = m_jackClient;

	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		auto clientName = std::string(jack_get_client_name(m_jackClient));
		fprintf (stderr, "unique name `%s' assigned\n", clientName.c_str());
	}
	
	
	jack_set_process_callback (m_jackClient, ITAJackProcess, &m_oUserData);
    jack_set_xrun_callback(m_jackClient, ITAJackXRUN, &m_oUserData);	
	jack_set_port_registration_callback(m_jackClient, ITAJackPortRegistered, &m_oUserData);	
	jack_on_shutdown (m_jackClient, ITAJackShutdown, 0);
	jack_set_port_connect_callback(m_jackClient, ITAJackPortConnected, &m_oUserData);

	m_iBufferSize = jack_get_buffer_size(m_jackClient);
	m_dSampleRate = jack_get_sample_rate(m_jackClient);
	
	// get physical ports
	const char** out_ports =  jack_get_ports(m_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	const char** in_ports =  jack_get_ports(m_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	
	m_iNumOutputChannels = 0;	
	m_iNumInputChannels = 0;

	while (out_ports[m_iNumOutputChannels]) m_iNumOutputChannels++;
	while (in_ports[m_iNumInputChannels++]) m_iNumInputChannels++;
	
	jack_free(out_ports);
	jack_free(in_ports);

	
	char port_name[12];
	for(i = 0; i < m_iNumOutputChannels; i++) {		
		sprintf(port_name, "out%00d", i);
		m_oUserData.output_ports[i] = jack_port_register (m_jackClient, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		if (m_oUserData.output_ports[i] == NULL) {
			fprintf(stderr, "no more JACK ports available\n");
			return ITA_JACK_ERROR;
		}
	}

	for(i = 0; i < m_iNumInputChannels; i++)
	{
		sprintf(port_name, "in%00d", i);
		m_oUserData.input_ports[i] = jack_port_register (m_jackClient, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		if (m_oUserData.input_ports[i] == NULL) {
			fprintf(stderr, "no more JACK ports available\n");
			return ITA_JACK_ERROR;
		}
	}
	
	//jack_set_sample_rate(m_jackClient, (jack_nframes_t)m_dSampleRate);


	/* install a signal handler to properly quits jack client */
#ifdef WIN32
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGTERM, signal_handler);
#else
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
#endif

	m_bInitialized = true;

	return ITA_JACK_NO_ERROR;
}

bool ITAJACKInterface::IsPlaybackEnabled() const {
	return m_bPlayback;
}

void ITAJACKInterface::SetPlaybackEnabled(const bool bEnabled) {
	m_bPlayback = bEnabled;
}

bool ITAJACKInterface::IsRecordEnabled() const {
	return m_bRecord;
}

void ITAJACKInterface::SetRecordEnabled(const bool bEnabled) {
	m_bRecord = bEnabled;
}

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Finalize() {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	if (m_bStreaming)
		return ITA_JACK_IS_STARTED;

	m_iError = ITA_JACK_NO_ERROR;

	for(int i = 0; i < m_iNumInputChannels; i++)
		jack_port_unregister(m_jackClient, m_oUserData.input_ports[i]);

	for(int i = 0; i < m_iNumOutputChannels; i++)
		jack_port_unregister(m_jackClient, m_oUserData.output_ports[i]);

    jack_client_close(m_jackClient);

	if (m_iError == ITA_JACK_NO_ERROR)
		m_bInitialized = false;

	if (m_oUserData.pdsRecordDatasource != NULL) {
		delete m_oUserData.pdsRecordDatasource;
		m_oUserData.pdsRecordDatasource = NULL;
	}
	
	return m_iError;
}

// Actually does nothing
ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Open() {
	if (!m_bInitialized) return ITA_JACK_NOT_INITIALIZED;
	if (m_bOpen) return ITA_JACK_IS_OPEN;
	if (m_oUserData.pdsPlaybackDatasource == NULL && m_bPlayback) return ITA_JACK_NO_PLAYBACK_DATASOURCE;
	if (m_oUserData.pdsRecordDatasource == NULL && m_bRecord) return ITA_JACK_NO_RECORD_DATASOURCE;


	if (m_bRecord) {
		if ((int) m_oUserData.pdsRecordDatasource->GetNumberOfChannels() > m_iNumInputChannels)
			return ITA_JACK_UNMATCHED_CHANNELS;

		m_oUserData.bRecord = true;
	}

	if (m_bPlayback) {
		if ((int) m_oUserData.pdsPlaybackDatasource->GetNumberOfChannels() > m_iNumOutputChannels)
			return ITA_JACK_UNMATCHED_CHANNELS;

		m_oUserData.bPlayback = true;
	}



	m_bOpen = true;

	return m_iError;
}

// Actually does nothing
ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Close() {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	if (!m_bOpen)
		return ITA_JACK_IS_NOT_OPEN;

	m_bOpen = false;

	return ITA_JACK_NO_ERROR;
}

bool ITAJACKInterface::connectJackPorts(bool connect)
{
	char portname[64]; // enough to hold all numbers up to 64-bits

	int i;
	for (i=0; i < m_iNumInputChannels; i++) {
		sprintf(portname, "system:capture_%d", i+1);
		if (connect ? jack_connect(m_jackClient, portname, jack_port_name(m_oUserData.input_ports[i]))
				 : jack_disconnect(m_jackClient, portname, jack_port_name(m_oUserData.input_ports[i]))) {
			fprintf(stderr,"Could not %s input ports %s and %s!\n", connect ? "connect" : "disconnect", portname, jack_port_name(m_oUserData.input_ports[i]));
            //return false;
		}
	}
	
	for (i=0; i < m_iNumOutputChannels; i++) {
		sprintf(portname, "system:playback_%d", i+1);
		if (connect ? jack_connect(m_jackClient, jack_port_name(m_oUserData.output_ports[i]), portname)
				 : jack_disconnect(m_jackClient, jack_port_name(m_oUserData.output_ports[i]), portname)) {
			fprintf(stderr,"Could not %s output ports %s and %s!\n", connect ? "connect" : "disconnect", jack_port_name(m_oUserData.output_ports[i]), portname);
            //return false;
		}
	}
	return true;
}

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Start() {
	if (m_bStreaming)
		return ITA_JACK_IS_STARTED;

	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	if (!m_bOpen)
		return ITA_JACK_IS_NOT_OPEN;


	/* Tell the JACK server that we are ready to roll.  Our
		* process() callback will start running now. */
	if (jack_activate (m_jackClient)) {
		return ITA_JACK_INTERNAL_ERROR;
	}

	if(!connectJackPorts(true))
		return ITA_JACK_UNMATCHED_CHANNELS;

	m_bStreaming = true;
	return ITA_JACK_NO_ERROR;
}

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::Stop() {
	if (!m_bStreaming)
		return ITA_JACK_STREAM_IS_STOPPED;

	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;
		
	if(!connectJackPorts(false))
		return ITA_JACK_UNMATCHED_CHANNELS;

	jack_deactivate(m_jackClient);
	m_bStreaming = false;

	return ITA_JACK_NO_ERROR;
}

int ITAJACKInterface::GetNumDevices() const {
	if (!m_bInitialized)
		return -1;
	return 1;
}

int ITAJACKInterface::GetNumInputChannels(const int iDeviceID) const {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;
	return m_iNumInputChannels;
}

int ITAJACKInterface::GetNumOutputChannels(const int iDeviceID) const {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;
	return m_iNumOutputChannels;
}

double ITAJACKInterface::GetSampleRate() const {
	return m_dSampleRate;
}

void ITAJACKInterface::GetNumChannels(const int iDeviceID, int& iNumInputChannels, int& iNumOutputChannels) const {
	iNumInputChannels = GetNumInputChannels(iDeviceID);
	iNumOutputChannels = GetNumOutputChannels(iDeviceID);
}

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::GetDriverSampleRate(const int iDeviceID, double& dSampleRate) const {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	dSampleRate = jack_get_sample_rate(m_jackClient);

	return ITA_JACK_NO_ERROR;
}

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::SetPlaybackDatasource(ITADatasource* pidsDatasource) {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	if (m_bOpen)
		return ITA_JACK_IS_OPEN;

	if (pidsDatasource->GetBlocklength() != m_iBufferSize)
		return ITA_JACK_UNMATCHED_BUFFER_SIZE;

	if (pidsDatasource->GetSampleRate() != m_dSampleRate)
		return ITA_JACK_UNMATCHED_SAMPLE_RATE;

	m_oUserData.pdsPlaybackDatasource = pidsDatasource;

	m_bPlayback = true;

	return ITA_JACK_NO_ERROR;
}

ITADatasource* ITAJACKInterface::GetRecordDatasource() {
	if (!m_bInitialized)
		return NULL;

	if (m_bOpen)
		return NULL;

	if (m_oUserData.pdsRecordDatasource == NULL)
		m_oUserData.pdsRecordDatasource = new ITAJackSource(m_iNumInputChannels, m_dSampleRate, m_iBufferSize);

	m_bRecord = true;

	return m_oUserData.pdsRecordDatasource;
}


std::string ITAJACKInterface::GetErrorCodeString(const ITA_JACK_ERRORCODE err) {
	if (err == ITA_JACK_NOT_INITIALIZED)
		return "JACK not initialized";
	if (err == ITA_JACK_UNMATCHED_CHANNELS)
		return "JACK channels are not matching";
	if (err == ITA_JACK_NO_PLAYBACK_DATASOURCE)
		return "JACK no datasource for playback available";
	if (err == ITA_JACK_NO_PLAYBACK_DATASOURCE)
		return "JACK no datasource for recording available";

	return "Unkown error code";
}

static int ITAJackProcess (jack_nframes_t nframes, void *arg)
{
#ifdef WITH_PROFILER
#error "JACK interface with profiler!"
	static bool threadReg = false;
	if(!threadReg) {
		printf("Registering Jack thread...\n");
		ProfilerRegisterThread();
		threadReg = true;
	}
#endif

	ITAJACKInterface::ITAJackUserData* userData = (ITAJACKInterface::ITAJackUserData*)arg;


	ITAJackSource* pipdsRecordDatasource = (ITAJackSource*) userData->pdsRecordDatasource;
	ITADatasource* pipdsPlaybackDatasource = userData->pdsPlaybackDatasource;
	bool bRecord = userData->bRecord;
	bool bPlayback = userData->bPlayback;



	ITAStreamInfo oStreamInfo;

	oStreamInfo.nSamples = userData->num_samples;

	// TODO: swap playback and record block here to avoid extra latency
	
	if (bPlayback) {
		int iNumOutputChannels = pipdsPlaybackDatasource->GetNumberOfChannels();
		const float* datablock;
		for (int j=0; j<iNumOutputChannels; j++) {
			jack_default_audio_sample_t *out = (jack_default_audio_sample_t *)jack_port_get_buffer (userData->output_ports[j], nframes);
			datablock = pipdsPlaybackDatasource->GetBlockPointer(j, &oStreamInfo);
			memcpy (out, datablock, sizeof (jack_default_audio_sample_t) * nframes);
		}
		pipdsPlaybackDatasource->IncrementBlockPointer();
	}	
	
	if (bRecord) {
		int iNumInputChannels = pipdsRecordDatasource->GetNumberOfChannels();
		float* datablock;
		for (int j=0; j<iNumInputChannels; j++) {
			jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (userData->input_ports[j], nframes);
			datablock = pipdsRecordDatasource->GetWritePointer(j);
			memcpy (datablock, in, sizeof (jack_default_audio_sample_t) * nframes);
		}
		pipdsRecordDatasource->IncrementWritePointer();
	}

	userData->num_samples += nframes;

	return 0;
}
