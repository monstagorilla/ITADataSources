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
// $Id: ITAStreamFunctionGenerator.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_FUNCTION_GENERATOR
#define INCLUDE_WATCHER_ITA_STREAM_FUNCTION_GENERATOR

#include <ITADataSourcesDefinitions.h>
#include <ITADataSourceRealization.h>

class ITA_DATA_SOURCES_API ITAStreamFunctionGenerator : public ITADatasourceRealization {
public:
	//! Signal functions
	enum {
		SINE = 0,		//!< Sine signal1
		TRIANGLE,		//!< Triangle signal
		SAWTOOTH,		//!< Sawtooth signal
		RECTANGLE,		//!< Rectangle signal (50% duty cycle)
		DIRAC,			//!< Dirac impulse(s)
		WHITE_NOISE		//!< White noise
	};

	//! Constructor
	/**
	 * Creates a new function generator.
	 *
	 * \param uiChannels		Number of output channels
	 * \param dSamplerate		Sampling rate [Hz]
	 * \param uiBlocklength		Blocklength [samples]
	 */
	ITAStreamFunctionGenerator(unsigned int uiChannels,
		                       double dSamplerate,
					           unsigned int uiBlocklength);

	//! Constructor
	/**
	 * Creates a new function generator with specific parameters
	 *
	 * \param uiChannels		Number of output channels
	 * \param dSamplerate		Sampling rate [Hz]
	 * \param uiBlocklength		Blocklength [samples]
	 * \param iSignalFunction	Signal function
	 * \param dFrequency		Signal frequency [Hz]
	 * \param dAmplitude		Signal amplitude
	 * \param bPeriodic			Generate a periodic signal?
	 */
	ITAStreamFunctionGenerator(unsigned int uiChannels,
		                       double dSamplerate,
					           unsigned int uiBlocklength,
							   int iSignalFunction,
							   double dFrequency,
							   float fAmplitude,
							   bool bPeriodic);

	//! Destructor
	virtual ~ITAStreamFunctionGenerator();

	//! Reset
	/**
	 * Resets all output streams.
	 */
	void Reset();

	//! Return the signal function
	int GetFunction() const;

	//! Sets the signal function
	void SetFunction(int iFunction);

	//! Returns the signal frequency [Hz]
	double GetFrequency() const;

	//! Sets the signal frequency [Hz]
	void SetFrequency(double dFrequency);

	//! Returns the signal period length [samples]
	int GetPeriodAsSamples() const;

	//! Sets the signal period length [samples]
	void SetPeriodAsSamples(int iNumSamples);

	//! Returns the signal period length [seconds]
	double GetPeriodAsTime() const;

	//! Sets the signal period length [seconds]
	void SetPeriodAsTime(double dPeriodLength);

	//! Return wheather periodic signal is generated
	bool IsPeriodic() const;

	//! Sets wheather periodic signal is generated
	void SetPeriodic(bool bPeriodic);

	//! Returns wheather the output is muted
	bool IsMuted() const;

	//! Muted/unmutes an the output
	void SetMuted(bool bMuted);

	//! Returns the output amplitude
	float GetAmplitude() const;

	//! Set the output gain
	void SetAmplitude(float fAmplitude);

private:
	ITAAtomicInt m_iFunction;
	ITAAtomicBool m_bPeriodic;
	ITAAtomicBool m_bMuted;
	ITAAtomicInt m_iNumSamples;
	ITAAtomicFloat m_fAmplitude;
	ITAAtomicInt m_iPeriodLengthSamples; //!< Number of samples per period
	float m_fPhase;			//!< Current phase information [radiants]
	int m_iSampleCount;		//!< Number of generated output samples
	
	// Generate the output samples
	void ProcessStream(const ITAStreamInfo* pStreamInfo);
};

#endif // INCLUDE_WATCHER_ITA_STREAM_FUNCTION_GENERATOR
