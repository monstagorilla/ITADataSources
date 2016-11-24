#include "ITAJACKInterface.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <signal.h>
#include <future>

#include <ITADataSource.h>
#include <ITADataSourceRealization.h>
#include <ITAStreamInfo.h>

#include <iostream>

static int ITAJackProcess (jack_nframes_t nframes, void *arg);
void connectNewPortToInput(jack_port_t * port, ITAJACKInterface::ITAJackUserData* userData);

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


ITAJACKInterface::ITAJACKInterface(int blockSize)
	:m_iBufferSize(blockSize)
{
	m_jackClient = NULL;

	m_iNumInputChannels = -1;
	m_iNumOutputChannels = -1;

	m_bInitialized = false;
	m_bOpen = false;
	m_bStreaming = false;
	
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

	if (m_iBufferSize > 0)
		jack_set_buffer_size(m_jackClient, m_iBufferSize);
	else
		m_iBufferSize = jack_get_buffer_size(m_jackClient);

	m_dSampleRate = jack_get_sample_rate(m_jackClient);
	
	// get physical ports
	const char** out_ports =  jack_get_ports(m_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	const char** in_ports =  jack_get_ports(m_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	
	m_iNumOutputChannels = 0;	
	m_iNumInputChannels = 0;

	while (out_ports[m_iNumOutputChannels]) m_iNumOutputChannels++;
	while (in_ports[m_iNumInputChannels]) m_iNumInputChannels++;
	
	jack_free(out_ports);
	jack_free(in_ports);




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

	auto func = connect ? &jack_connect : &jack_disconnect;

	int i;
	for (i=0; i < (std::min)((size_t)m_iNumInputChannels, m_oUserData.input_ports.size()); i++) {
		sprintf(portname, "system:capture_%d", i+1);
		if (func(m_jackClient, portname, jack_port_name(m_oUserData.input_ports[i]))) {
			fprintf(stderr,"Could not %s input ports %s and %s!\n", connect ? "connect" : "disconnect", portname, jack_port_name(m_oUserData.input_ports[i]));
		}
	}
	
	for (i=0; i < (std::min)((size_t)m_iNumOutputChannels, m_oUserData.output_ports.size()); i++) {
		sprintf(portname, "system:playback_%d", i+1);
		if (connect ? jack_connect(m_jackClient, jack_port_name(m_oUserData.output_ports[i]), portname)
				 : jack_disconnect(m_jackClient, jack_port_name(m_oUserData.output_ports[i]), portname)) {
			fprintf(stderr,"Could not %s output ports %s and %s!\n", connect ? "connect" : "disconnect", jack_port_name(m_oUserData.output_ports[i]), portname);
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
		std::cerr << "Failed to activate jack client!" << std::endl;
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

ITAJACKInterface::ITA_JACK_ERRORCODE ITAJACKInterface::SetPlaybackDatasource(ITADatasource* pidsDatasource) {
	if (!m_bInitialized)
		return ITA_JACK_NOT_INITIALIZED;

	if (m_bOpen)
		return ITA_JACK_IS_OPEN;

	if (pidsDatasource->GetBlocklength() != m_iBufferSize)
		return ITA_JACK_UNMATCHED_BUFFER_SIZE;

	if (pidsDatasource->GetSampleRate() != m_dSampleRate)
		return ITA_JACK_UNMATCHED_SAMPLE_RATE;

	if (static_cast<int>(pidsDatasource->GetNumberOfChannels()) > m_iNumInputChannels)
		return ITA_JACK_UNMATCHED_CHANNELS;

	// register ports with first playback datasource
	if (!m_oUserData.pdsPlaybackDatasource) {
		char port_name[20];
		for (int i = 0; i < m_iNumOutputChannels; i++) {
			sprintf(port_name, "out%00d", i);
			auto port = jack_port_register(m_jackClient, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			if (!port) {
				std::cerr << "no more JACK ports available" << std::endl;
				return ITA_JACK_ERROR;
			}
			m_oUserData.output_ports.push_back(port);
		}
	}

	m_oUserData.pdsPlaybackDatasource = pidsDatasource;

	return ITA_JACK_NO_ERROR;
}

ITADatasource* ITAJACKInterface::GetRecordDatasource() {
	if (!m_bInitialized)
		return NULL;

	if (m_bOpen)
		return NULL;

	if (m_oUserData.pdsRecordDatasource == NULL) {

		// create input ports
		for (int i = 0; i < m_iNumInputChannels; i++)
		{
			char port_name[20];
			sprintf(port_name, "in%00d", i);
			auto port = jack_port_register(m_jackClient, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
			m_oUserData.input_ports.push_back(port); 
			if (port == NULL) {
				fprintf(stderr, "no more JACK ports available\n");
				return NULL;
			}
		}

		m_oUserData.pdsRecordDatasource = new ITAJackSource(m_iNumInputChannels, m_dSampleRate, m_iBufferSize);
	}
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

void ITAJACKInterface::printInfo() {
	std::cout << "JACK initialized" << std::endl;
	std::cout << "  sampling rate: " << GetSampleRate() << std::endl;
	std::cout << "  block size: " << GetBlockSize() << std::endl;
	std::cout << "  input  ch#: " << m_iNumInputChannels << std::endl;
	std::cout << "  output ch#: " << m_iNumOutputChannels << std::endl;
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


	ITAStreamInfo oStreamInfo;

	oStreamInfo.nSamples = userData->num_samples;

	// TODO: swap playback and record block here to avoid extra latency
	if (pipdsRecordDatasource) {
		int iNumInputChannels = pipdsRecordDatasource->GetNumberOfChannels();
		float* datablock;
		for (int j=0; j<iNumInputChannels; j++) {
			jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (userData->input_ports[j], nframes);
			datablock = pipdsRecordDatasource->GetWritePointer(j);
			memcpy (datablock, in, sizeof (jack_default_audio_sample_t) * nframes);
		}
		pipdsRecordDatasource->IncrementWritePointer();
	}

	if (pipdsPlaybackDatasource) {
		int iNumOutputChannels = pipdsPlaybackDatasource->GetNumberOfChannels();
		const float* datablock;
		for (int j = 0; j<iNumOutputChannels; j++) {
			jack_default_audio_sample_t *out = (jack_default_audio_sample_t *)jack_port_get_buffer(userData->output_ports[j], nframes);
			datablock = pipdsPlaybackDatasource->GetBlockPointer(j, &oStreamInfo);
			memcpy(out, datablock, sizeof(jack_default_audio_sample_t) * nframes);
		}
		pipdsPlaybackDatasource->IncrementBlockPointer();
	}

	userData->num_samples += nframes;

	return 0;
}


// Callbacks

/*
this function handles new port registrations from other clients
*/
void ITAJackPortRegistered(jack_port_id_t port_id, int reg, void *arg)
{
	ITAJACKInterface::ITAJackUserData* userData = (ITAJACKInterface::ITAJackUserData*)arg;	
	jack_port_t * port = jack_port_by_id (ITAJACKInterface::GetJackClient(), port_id);

	// ignore own ports
	if(jack_port_is_mine(ITAJACKInterface::GetJackClient(), port)) {
		return;
	}
	
	// ignore mplayer!
	if(strstr(jack_port_name (port), "MPlayer") != NULL) {
		return;
	}

	int flags = jack_port_flags(port);

	if(flags & JackPortIsOutput) {
		std::async(std::launch::async, [port, userData] {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			connectNewPortToInput(port, userData);
		});
	}

	/*
	do_http_callback("port_registered",
		"port_name=%s", jack_port_name (port),
		"register=%d", reg,
		"flags=%d", jack_port_flags(port),
		NULL
	);*/
}

int ITAJackXRUN(void *arg)
{
	ITAJACKInterface::ITAJackUserData* userData = (ITAJACKInterface::ITAJackUserData*)arg;
	userData->num_xruns++;
	
	if((userData->num_xruns % 100) == 0)
	//	fprintf(stderr,"XRUNS: %lu\n",userData->num_xruns);
	
    return 0;
}



void ITAJackMsg(const char *msg)
{
	fprintf(stderr,"%s\n",msg);
}

void ITAJackShutdown (void *arg)
{
	//if(arg != NULL)
//		delete arg;
	fprintf(stderr, "ITAJackShutdown: exiting ...\n");
	exit(0);
}



void connectNewPortToInput(jack_port_t * port, ITAJACKInterface::ITAJackUserData* userData)
{
	const char *portNameA = jack_port_name(port);

	// get channel number by trailing number (-1 because 0 indexing)
	int ch = atoi(&portNameA[strlen(portNameA) - 1]) - 1;
	if (ch >= 0 && ch < userData->input_ports.size()) {
		if (!jack_port_connected_to(userData->input_ports[ch], portNameA)) {
			printf("Connecting port %s -> %s\n", portNameA, jack_port_name(userData->input_ports[ch]));
			if (jack_connect(ITAJACKInterface::GetJackClient(), portNameA, jack_port_name(userData->input_ports[ch])) != 0) {
				printf("\t... connection failed!\n");
			}
		}
	}

	// if mono (e.g. no numeric port suffix), connect to all ports!
	if (ch == -1) {
		for (ch = 0; ch < userData->input_ports.size(); ch++) {
			if (userData->input_ports[ch] && !jack_port_connected_to(userData->input_ports[ch], portNameA)) {
				printf("Connecting port %s -> %s\n", portNameA, jack_port_name(userData->input_ports[ch]));
				if (jack_connect(ITAJACKInterface::GetJackClient(), portNameA, jack_port_name(userData->input_ports[ch])) != 0)
					printf("\t... connection failed!\n");
			}
		}
	}
}



void ITAJackPortConnected(jack_port_id_t a, jack_port_id_t b, int connect, void* arg)
{
	ITAJACKInterface::ITAJackUserData* userData = (ITAJACKInterface::ITAJackUserData*)arg;

	jack_client_t *client = ITAJACKInterface::GetJackClient();

	jack_port_t * portA = jack_port_by_id (client, a);
	jack_port_t * portB = jack_port_by_id (client, b);

	// ignore if any of the port belongs to the client
	if(jack_port_is_mine(client, portA) || jack_port_is_mine(client, portB) ) {
		return;
	}

	//int flagsA = jack_port_flags(portA);
	int flagsB = jack_port_flags(portB);

	if(flagsB & JackPortIsPhysical) {

		const char *portNameA = jack_port_name(portA);
		
		std::async(std::launch::async, [client, portNameA, portA, portB, userData] {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			jack_disconnect(client, portNameA, jack_port_name(portB));
			connectNewPortToInput(portA, userData);
		});
	}
}

