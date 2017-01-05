/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2017
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
// $Id: ITAPortaudioInterface.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_PORTAUDIO_INTERFACE
#define INCLUDE_WATCHER_ITA_PORTAUDIO_INTERFACE

#include <ITADataSourcesDefinitions.h>

#include <string>
#include <vector>

// Forward declaration of ITADatasource
class ITADatasource;

/*! ITAPortaudioInterface
 *
 * This class provides an eays-to-use connection and configuration to Portaudio. It
 * gives opportunity to playback and record ITADatasources.
 *
 * see also \ITADatasources
 *
 */
class ITA_DATA_SOURCES_API ITAPortaudioInterface
{
public:
	//! ITAPortaudio error code table
	typedef enum ITA_PA_ERRORCODE
	{
		//! Portaudio/ITAPortaudio no error
		ITA_PA_NO_ERROR=0,

		//! ITAPortaudio invalid configuration (deprecated)
		ITA_PA_INVALID_CONFIGURATION = -10,

		//! ITAPortaudio already initialized
		ITA_PA_IS_INITIALIZED,

		//! ITAPortaudio stream is open
		ITA_PA_IS_OPEN,

		//! ITAPortaudio stream is not open
		ITA_PA_IS_NOT_OPEN,

		//! ITAPortaudio no playback data source provided
		ITA_PA_NO_PLAYBACK_DATASOURCE,

		//! ITAPortaudio no playback data source provided
		ITA_PA_NO_RECORD_DATASOURCE,

		//! ITAPortaudio buffer size is not matching
		ITA_PA_UNMATCHED_BUFFER_SIZE,

		//! ITAPortaudio sample rate is not matching
		ITA_PA_UNMATCHED_SAMPLE_RATE,

		//! ITAPortaudio channels are not matching
		ITA_PA_UNMATCHED_CHANNELS,

		//! Portaudio not initialized
		ITA_PA_NOT_INITIALIZED = -10000,

		//! Portaudio unanticipated host error
		ITA_PA_UNANTICIPATED_HOST_ERROR,

		//! Portaudio invalid channel count
		ITA_PA_INVALID_CHANNEL_COUNT,

		//! Portaudio invalid sample rate
		ITA_PA_INVALID_SAMPLE_RATE,

		//! Portaudio invalid device
		ITA_PA_INVALID_DEVICE,

		//! Portaudio invalid flag
		ITA_PA_INVALID_FLAG,

		//! Portaudio invalid sample format
		ITA_PA_SAMPLE_FORMAT_NOT_SUPPORTED,

		//! Portaudio bad input output device combination
		ITA_PA_BAD_IO_DEVICE_COMBINATION,

		//! Portaudio insufficient memobry
		ITA_PA_INSUFFICIENT_MEMORY,

		//! Portaudio buffer too big
		ITA_PA_BUFFER_TOO_BIG,

		//! Portaudio buffer too small
		ITA_PA_BUFFER_TOO_SMALL,

		//! Portaudio null callback
		ITA_PA_NULL_CALLBACK,

		//! Portaudio bad stream pointer
		ITA_PA_BAD_STREAM_POINTER,

		//! Portaudio timeout
		ITA_PA_TIMED_OUT,

		//! Portaudio/ITAPortaudio internal error
		ITA_PA_INTERNAL_ERROR,

		//! Portaudio device unavailable
		ITA_PA_DEVICE_UNAVAILABLE,

		//! Portaudio incompatible host api specific stream info
		ITA_PA_INCOMPATOBLE_HOST_API_SPECIFIC_STREAM_INFO,

		//! Portaudio stream is stopped
		ITA_PA_STREAM_IS_STOPPED,

		//! Portaudio stream is started
		ITA_PA_IS_STARTED,

		//! Portaudio input overflowed
		ITA_PA_INPUT_OVERFLOWED,

		//! Portaudio output underflowed
		ITA_PA_OUTPUT_UNDERFLOWED,

		//! Portaudio host api not found
		ITA_PA_HOST_API_NOT_FOUND,

		//! Portaudio invalid host api
		ITA_PA_INVALID_HOST_API,

		//! Portaudio can not read from a callback stream
		ITA_PA_CAN_NOT_READ_FROM_A_CALLBACK_STREAM,

		//! Portaudio can not write to a callback stream
		ITA_PA_CAN_NOT_WRITE_TO_A_CALLBACK_STREAM,

		//! Portaudio can not read from an output only stream
		ITA_PA_CAN_NOT_READ_FROM_AN_OUTPUT_ONLY_STREAM,

		//! Portaudio can not write to an input only stream
		ITA_PA_CAN_NOT_WRITE_TO_AN_INPUT_ONLY_STREAM,

		//! Portaudio incompatible stream host api
		ITA_PA_INCOMPATIBLE_STREAM_HOST_API,

		//! Portaudio bad buffer pointer
		ITA_PA_BAD_BUFFER_PTR
	};

	//! Portaudio available host APIs
	typedef enum ITA_PA_HOST_APIS
	{
		ITA_PA_DIRECT_SOUND = 1,	//!< Windows DirectSound
		ITA_PA_MME = 2,				//!< Windows MME
		ITA_PA_ASIO = 3,			//!< Windows Steinberg ASIO (recommended: use ITAsioInterface instead)
		ITA_PA_SOUND_MANAGER = 4,	//!< Macintosh Sound Manager
		ITA_PA_CORE_AUDIO = 5,		//!< MacOS CoreAudio
		ITA_PA_OSS = 7,				//!< Linux/Unix OSS
		ITA_PA_ALSA = 8,			//!< Linux/Unix ALSA
		ITA_PA_AL = 9,				//!< Silicon Graphics Irix using AL
		ITA_PA_BE_OS = 10,			//!< BeOS
		ITA_PA_WDMKS = 11,			//!< Windows Driver Model Kernel Streaming driver
		ITA_PA_JACK = 12,			//!< MacOS/Linux/Unix Jack Audio
		ITA_PA_WASAPI = 13,			//!< Windows Audio Session API
		ITA_PA_AUDIO_SCIENCE_HPI=14	//!< AudioScience Hardware Programming Interface
	};


	//! Constructor with sample rate and buffer size
	/**
	  * Set up internal variables of ITAPortaudio. No exception will be
	  * thrown here.
	  * @note Next do initialization
	  *
	  * @see Initialize()
	  */
	ITAPortaudioInterface( double dSampleRate, int iBufferSize );

	//! Destructor
	~ITAPortaudioInterface();

	//! Initialize Portaudio using default hardware and default host/driver
	/**
	  * Initializes Portaudio with the current driver. If no driver has been set,
	  * the default output device will be used, while the input device will be
	  * deactivated (playback mode on, recording mode off).
	  * 
	  * \return Will return error code if Portaudio could not be initialized with the current configuration, ITA_PA_NO_ERROR otherwise
	  */
	ITA_PA_ERRORCODE Initialize();

	//! Initialize Portaudio using specified host/driver by id
	ITA_PA_ERRORCODE Initialize( int iDriverID );

	//! Initialize Portaudio using specified driver by name
	ITA_PA_ERRORCODE Initialize( const std::string& sDriverName );

	//! Use Portaudio with specific input device
	ITA_PA_ERRORCODE SetOutputDevice( int iOutputDevice );

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
	ITA_PA_ERRORCODE Finalize();

	//! Opens a Portaudio stream
	ITA_PA_ERRORCODE Open();

	//! Closes the Portaudio stream
	ITA_PA_ERRORCODE Close();

	//! Start Portaudio streaming
	ITA_PA_ERRORCODE Start();

	//! Stop Portaudio streaming
	ITA_PA_ERRORCODE Stop();

	//! Returns the number of drivers found by Portaudio
	int GetNumDevices() const;

	//! Returns the name of the driver avaiable in Portaudio
	std::string GetDeviceName( int iDriverID ) const;

	static int GetPreferredBufferSize();

	//! Returns the interactive low latency capability of the driver
	/**
	  * \param iDriverID Identifier of driver
	  * \return Latency in seconds, -1 if any error with the driver occurs
	  */
	float GetDeviceLatency( int iDriverID ) const;

	ITA_PA_ERRORCODE GetDriverSampleRate( int iDeviceID, double& dSampleRate ) const;

	//! Returns the name of the current devices in Portaudio
	std::string GetInputDeviceName() const;

	//! Returns the name of the current devices in Portaudio
	std::string GetOutputDeviceName() const;

	//! Get default input device index
	int GetDefaultInputDevice() const;

	//! Get default output device index
	int GetDefaultOutputDevice() const;

	//! Get current input device index
	int GetInputDevice() const;

	//! Get current output device index
	int GetOutputDevice() const;

	//! Returns the number of input and output channels
	void GetNumChannels( int iDeviceID, int& iNumInputChannels, int& iNumOutputChannels ) const;

	//! Returns the number of input channels
	/**
	  * \return Number of input channels (>=0) or #ITA_PA_ERRORCODE (<0)
	  */
	int GetNumInputChannels( int iDeviceID ) const;

	//! Returns the number of output channels
	/**
	  * \return Number of output channels (>=0) or #ITA_PA_ERRORCODE (<0)
	  */
	int GetNumOutputChannels( int iDeviceID ) const;

	//! Returns the sample rate
	double GetSampleRate() const;

	//! Sets the sample rate
	ITA_PA_ERRORCODE SetSampleRate( double dSampleRate );
	
	//! Set the playback data source
	/**
	  * \note Enables playback, see IsPlaybackEnabled() and SetPlaybackEnabled()
	  */
	ITA_PA_ERRORCODE SetPlaybackDatasource( ITADatasource* pidsDatasource );

	//! Get the recording data source
	/**
	  * This also creates the record datasource if not already present.
	  
	  * \note Enables recording, see IsRecordingEnabled() and SetRecordingEnabled()
	  *
	  * \see Finalize()
	  */
	ITADatasource* GetRecordDatasource();

	//! Uses the Portaudio sleep function
	void Sleep( float fSeconds ) const;

	//! Returns a human readable error code string
	static std::string GetErrorCodeString( ITA_PA_ERRORCODE err );
	

	//! Internal user data class for information exchange with callback function
	class ITAPortaudioUserData
	{
	public:
		ITADatasource* pdsPlaybackDatasource; //!< ITADatasource playback datasource
		ITADatasource* pdsRecordDatasource;   //!< ITADatasource record datasource
		bool bPlayback;	//!< Playback enabled
		bool bRecord;   //!< Record enabled

		inline ITAPortaudioUserData()
		{
			pdsPlaybackDatasource = NULL;
			pdsRecordDatasource = NULL;
			bPlayback = false;
			bRecord = false;
		}
	};

private:
	//! Standard constructor deactivated
	ITAPortaudioInterface();
	
	std::string m_sConfigFile;	//!< Configuration file path
	double m_dSampleRate;		//!< Internal sampling rate
	int m_iBufferSize;			//!< Internal buffer size

	void* m_vpPaStream;			//!< Portaudio stream pointer

	ITAPortaudioUserData m_oUserData;		//!< ITAPortaudioDatasource user data
	
	bool m_bInitialized;		//!< Portaudio initialization status
	bool m_bOpen;				//!< Portaudio open status
	bool m_bStreaming;			//!< Portaudio streaming status
		
	bool m_bRecord;				//!< Portaudio recording mode
	bool m_bPlayback;			//!< Portaudio playback mode
	
	int m_iNumInputChannels;	//!< Number of input channels
	int m_iNumOutputChannels;	//!< Number of output channels

	int m_iDriverID;			//!< Portaudio driver identifier
	/*
	int m_iInputDevice;			//!< Identifier of input device
	int m_iOutputDevice;		//!< Identifier of output device
	*/

	ITA_PA_ERRORCODE m_iError;	//!< Last ITAPortaudio error

};

#endif // INCLUDE_WATCHER_ITA_PORTAUDIO_INTERFACE
