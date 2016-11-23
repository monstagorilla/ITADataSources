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
#include <jack/jack.h>



#ifndef JACK_MAX_CHANNELS
	#define JACK_MAX_CHANNELS 16
#endif



#include <functional>

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

	ITAJACKInterface(int blockSize = -1);

	//! Destructor
	~ITAJACKInterface();


	//! Initialize JACK
	ITA_JACK_ERRORCODE Initialize(const std::string& clientName);


	//! Returns true if playback is enabled, false otherwise
	bool IsPlaybackEnabled() const;

	//! Set playback enabled/disabled
	void SetPlaybackEnabled( bool bEnabled);

	//! Returns true if record is enabled, false otherwise
	bool IsRecordEnabled() const;

	//! Set record enabled/disabled
	void SetRecordEnabled( bool bEnabled);

	//! Finalize Portaudio
	/**
	  * This also deletes the record datasource.
	  */
	ITA_JACK_ERRORCODE Finalize();

	//! Opens a Portaudio stream
	ITA_JACK_ERRORCODE Open();

	//! Closes the Portaudio stream
	ITA_JACK_ERRORCODE Close();

	//! Start Portaudio streaming
	ITA_JACK_ERRORCODE Start();

	//! Stop Portaudio streaming
	ITA_JACK_ERRORCODE Stop();

	//! Returns the number of drivers found by Portaudio
	int GetNumDevices() const;

	//! Returns the name of the driver avaiable in Portaudio
	std::string GetDeviceName( int iDriverID ) const;


	//! Returns the interactive low latency capability of the driver
	/**
	  * \param iDriverID Identifier of driver
	  * \return Latency in seconds, -1 if any error with the driver occurs
	  */
	float GetDeviceLatency() const;

	ITA_JACK_ERRORCODE GetDriverSampleRate(int iDeviceID, double& dSampleRate) const;


	//! Get current input device index
	int GetInputDevice() const;

	//! Get current output device index
	int GetOutputDevice() const;

	//! Returns the number of input and output channels
	void GetNumChannels(int iDeviceID, int& iNumInputChannels, int& iNumOutputChannels) const;

	//! Returns the number of input channels
	/**
	  * \return Number of input channels (>=0) or #ITA_JACK_ERRORCODE (<0)
	  */
	int GetNumInputChannels(int iDeviceID=0) const;

	//! Returns the number of output channels
	/**
	  * \return Number of output channels (>=0) or #ITA_JACK_ERRORCODE (<0)
	  */
	int GetNumOutputChannels(int iDeviceID) const;

	//! Returns the sample rate
	double GetSampleRate() const;

	//! Sets the sample rate
	ITA_JACK_ERRORCODE SetSampleRate(double dSampleRate);
	
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



	
	struct ITAJackUserData {
		ITADatasource* pdsPlaybackDatasource; //!< ITADatasource playback datasource
		ITADatasource* pdsRecordDatasource;   //!< ITADatasource record datasource		

		bool bPlayback;	//!< Playback enabled
		bool bRecord;   //!< Record enabled
		jack_port_t *input_ports[JACK_MAX_CHANNELS];
		jack_port_t *output_ports[JACK_MAX_CHANNELS];
		uint64_t num_xruns;
		uint64_t num_samples;
		
	


		ITAJackUserData() 
		{
			pdsPlaybackDatasource = NULL;
			pdsRecordDatasource = NULL;
			bPlayback = false;
			bRecord = false;
			memset(input_ports, 0, sizeof(input_ports));
			memset(output_ports, 0, sizeof(output_ports));
			num_xruns = 0;
			num_samples = 0;
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
		
	bool m_bRecord;				//!< Portaudio recording mode
	bool m_bPlayback;			//!< Portaudio playback mode
	
	int m_iNumInputChannels;	//!< Number of input channels
	int m_iNumOutputChannels;	//!< Number of output channels

	ITA_JACK_ERRORCODE m_iError;	//!< Last error
	
	bool connectJackPorts(bool connect);
};

