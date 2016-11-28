/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2016
*
* ----------------------------------------------------------------
*				    ____  __________  _______
*				   //  / //__   ___/ //  _   |
*				  //  /    //  /    //  /_|  |
*				 //  /    //  /    //  ___   |
*				//__/    //__/    //__/   |__|
*
* ----------------------------------------------------------------
*
*/
// $Id: ITAJACKInterface.h $

#pragma once

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <queue> 
#include <functional>

#include <jack/jack.h>

// Forward declaration of ITADatasource
class ITADatasource;


// Forward declarations of callbacks
int ITAJackXRUN(void *arg);
void ITAJackMsg(const char *msg);
void ITAJackShutdown (void *arg);
void ITAJackPortConnected(jack_port_id_t a, jack_port_id_t b, int connect, void* arg);
void ITAJackPortRegistered(jack_port_id_t port_id, int reg, void *arg);


/*! ITAJACKInterface
 *
 * 
 *
 * see also \ITADatasources
 *
 */
class ITAJACKInterface {
public:
	//! ITAPortaudio error code table
	enum ITA_JACK_ERRORCODE {
		//! Portaudio/ITAPortaudio no error
		ITA_JACK_NO_ERROR=0,
		ITA_JACK_ERROR=1,
		ITA_JACK_IS_OPEN,
		ITA_JACK_IS_STARTED,
		ITA_JACK_OPEN_FAILED,
		ITA_JACK_NOT_INITIALIZED,
		ITA_JACK_UNMATCHED_CHANNELS,
		ITA_JACK_NO_PLAYBACK_DATASOURCE,
		ITA_JACK_NO_RECORD_DATASOURCE,
		ITA_JACK_IS_NOT_OPEN,
		ITA_JACK_INTERNAL_ERROR,
		ITA_JACK_STREAM_IS_STOPPED,
		ITA_JACK_UNMATCHED_BUFFER_SIZE,
		ITA_JACK_UNMATCHED_SAMPLE_RATE

	};

	//! If no blockSize given the interface will use blockSize of JACK server
	ITAJACKInterface(int blockSize = -1);
	~ITAJACKInterface();


	//! Initialize JACK
	ITA_JACK_ERRORCODE Initialize(const std::string& clientName);


	//! Finalize JACK
	/**
	  * This also deletes the record datasource.
	  */
	ITA_JACK_ERRORCODE Finalize();

	//! Opens a JACK client
	ITA_JACK_ERRORCODE Open();

	//! Closes the JACK client
	ITA_JACK_ERRORCODE Close();

	//! Start JACK client
	ITA_JACK_ERRORCODE Start();

	//! Stop JACK client
	ITA_JACK_ERRORCODE Stop();

	inline int GetNumInputChannels() const {
		return m_bInitialized ? m_iNumInputChannels : 0;
	}
	inline int GetNumOutputChannels() const {
		return m_bInitialized ? m_iNumOutputChannels : 0;
	}

	inline int GetSampleRate() const {
		return m_dSampleRate;
	}

	inline int GetBlockSize() const { return m_iBufferSize; }

	inline int SetBlockSize(int blockSize);

	
	//! Set the playback data source
	/**
	  * \note Enables playback, see IsPlaybackEnabled() and SetPlaybackEnabled()
	  */
	ITA_JACK_ERRORCODE SetPlaybackDatasource(ITADatasource* pidsDatasource);

	//! Get the recording data source
	/**
	  * This also creates the record datasource if not already present.
	  
	  * \note Enables recording, see IsRecordingEnabled() and SetRecordingEnabled()
	  *
	  * \see Finalize()
	  */
	ITADatasource* GetRecordDatasource();

	//! Returns a human readable error code string
	static std::string GetErrorCodeString(ITA_JACK_ERRORCODE err);
	
	inline static jack_client_t *GetJackClient() { return s_jackClient; }

	void printInfo();
	
	struct ITAJackUserData {
		ITADatasource* pdsPlaybackDatasource; //!< ITADatasource playback datasource
		ITADatasource* pdsRecordDatasource;   //!< ITADatasource record datasource
		std::vector<jack_port_t *> input_ports, output_ports;
		uint64_t num_xruns;
		uint64_t num_samples;

		ITAJackUserData() : num_xruns(0), num_samples(0), pdsPlaybackDatasource(nullptr), pdsRecordDatasource(nullptr)
		{
		}
	};
	

private:
	static jack_client_t *s_jackClient;
	jack_client_t *m_jackClient;
	

	int m_iBufferSize;
	double m_dSampleRate;

	ITAJackUserData m_oUserData;		//!< ITAPortaudioDatasource user data
	
	bool m_bInitialized;		//!< Portaudio initialization status
	bool m_bOpen;				//!< Portaudio open status
	bool m_bStreaming;			//!< Portaudio streaming status
		
	
	int m_iNumInputChannels;	//!< Number of input channels
	int m_iNumOutputChannels;	//!< Number of output channels

	ITA_JACK_ERRORCODE m_iError;	//!< Last error
	
	bool connectJackPorts(bool connect);
};

