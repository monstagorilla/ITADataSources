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
// $Id: ITAStreamPatchBay.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_PATCH_BAY
#define INCLUDE_WATCHER_ITA_STREAM_PATCH_BAY

#include <ITADataSourcesDefinitions.h>
       
#include <ITAAtomicPrimitives.h>
#include <ITADataSourceRealization.h>
#include <ITAUncopyable.h>
#include <set>
#include <string>
#include <vector>

/*  Usage example:

	ITAStreamPatchbay* pb = new ITAStreamPatchbay(44100, 128);
	pb->AddInput(1);
	pb->AddInput(3);
	pb->AddInput(2);
	pb->AddOutput(4);

	pb->Connect(0,0,0,1);
	pb->Connect(2,0,0,2);

	pb->Clear();

*/

//! Patchbay for audio streams
/** 
 * TODO: Documentation
 * TODO: Crossfading für Parameter-Änderungen ist nicht sauber implementiert.
 */

class ITA_DATA_SOURCES_API ITAStreamPatchbay : public ITADatasourceRealizationEventHandler, public ITAUncopyable
{
public:
	//! Strategien zur Lautstärke-Anpassung
	enum
	{
		GAIN_SWITCH			=0,	//!< Hartes umschalten der Verstärkung
		GAIN_LINEAR_FADE		//!< Lineare Anpassung innerhalb eines Stream-Blocks
	};

	//! Constructor
	/**
	 * Creates a new patchbay without any inputs and outputs.
	 *
	 * \param dSamplerate		Sampling rate [Hz]
	 * \param iBlockLength		Blocklength [samples]
	 */
	ITAStreamPatchbay( const double dSamplerate,
					   const int iBlockLength,
					   const int iFadeLength=128,
					   const int iGainAdaption=GAIN_LINEAR_FADE );

	//! Destructor
	virtual ~ITAStreamPatchbay();

	//! Reset
	/**
	 * Resets the streams of all outputs and clears internal buffers.
	 *
	 * \important Do not call when streaming
	 */
	void Reset();

	//! Remove all inputs, outputs and channel routings
	/*
	 * \important Do not call when streaming
	 */
	void Clear();

	//! Returns the number of inputs
	int GetNumInputs() const;

	//! Returns the number of outputs
	int GetNumOutputs() const;

	//! Returns the number of channels of an input
	int GetInputNumChannels( const int iInput ) const;

	//! Returns the number of channels of an output
	int GetOutputNumChannels(const int iOutput ) const;

	//! Creates a new input and returns its index
	/*
	 * \important Do not call when streaming
	 */
	int AddInput( const int iNumChannels );

	//! Creates a new input, connects the given datasource and returns input index
	/*
	 * \important Do not call when streaming
	 */
	int AddInput( ITADatasource* pIn );

	//! Removes an input
	/*
	 * \important Do not call when streaming
	 * \note All subsequent input IDs will be decreased
	 */
	bool RemoveInput( int uiInput );

	//! Creates a new output and returns its index
	/*
	 * \important Do not call when streaming
	 */
	int AddOutput( const int iNumChannels );

	//! Return the datasource of an input
	/**
	 * \note Returns NULL if no datasource is attached to this input
	 */
	ITADatasource* GetInputDatasource( const int iInput ) const;

	//! Set the datasource for an input
	/**
	 * \note Passing NULL removes a present datasource assignment
	 * \important Do not call when streaming
	 */
	void SetInputDatasource( const int iInput, ITADatasource* pdsDatasource );

	//! Return the datasource of an output
	ITADatasource* GetOutputDatasource( const int iOutput ) const;

	/* Evtl. later:
	//! For an output return the connected input (-1 => output not connected)
	void GetOutputConnectedToInput(const int iInput,
		                           const int iInputChannel,
			                       int& iOutput,
			                       int& iOutputChannel);

	//! For an input return the connected output (-1 => input not connected)
	void GetInputConnectedToOutput(const int iOutput,
		                           const int iOutputChannel,
			                       int& iInput,
			                       int& iInputChannel);
	*/

	//! Connect an input channel to an output channel
	void ConnectChannels( const int iInput,
		                  const int iInputChannel,
				          const int iOutput,
		                  const int iOutputChannel );

	//! Connect an input channel to an output channel with a gain
	void ConnectChannels( const int iInput,
		                 const int iInputChannel,
				         const int iOutput,
		                 const int iOutputChannel,
						 const double dGain );

	//! Disconnect one input of an output
	void Disconnect( const int iInput,
					 const int iOutput );

	//! Disconnect all channels of an output
	void DisconnectOutput( const int iOutput );

	//! Disconnect a specific channel of an output
	void DisconnectOutputChannel( const int iOutput,
		                          const int iOutputChannel );

	//! Remove all connections between all inputs and all outputs
	void DisconnectAllOutputs();

	//! Returns wheather an input is muted
	bool IsInputMuted( const int iInput ) const;

	//! Mutes/unmutes an input
	void SetInputMuted( const int iInput, const bool bMuted );

	//! Returns wheather an output is muted
	bool IsOutputMuted( const int iOutput ) const;

	//! Mutes/unmutes an output
	void SetOutputMuted( const int iOutput, const bool bMuted );

	//! Returns the gain of an input
	double GetInputGain( const int iInput ) const;

	//! Set the gain of an input
	void SetInputGain( const int iInput, const double dGain );

	//! Returns the gain of an output
	double GetOutputGain( const int iOutput ) const;

	//! Set the gain of an output
	void SetOutputGain( const int iOutput, const double dGain );

	//! Returns the gain of an input-output connection
	double GetConnectionGain( const int iInput,
		                      const int iInputChannel,
				              const int iOutput,
		                      const int iOutputChannel ) const;

	//! Set the gain of an input-output connection
	void SetConnectionGain( const int iInput,
		                    const int iInputChannel,
				            const int iOutput,
		                    const int iOutputChannel,
						    const double dGain );

	//! Return the method for gain adaption
	unsigned int GetGainAdaption() const;

	//! Sets the method for gain adaption
	void SetGainAdaption( const int iGainAdaption );

    //! Returns the number of samples used for gain fading
	unsigned int GetGainFadeLength() const;
	
    //! Sets the number of samples for gain fading
	/**
	 * \param iFadeLength  Number of samples for fading gains
	 *
	 * \note Maximum number of samples is the blocklength
	 */
	void SetGainFadeLength( const int iFadeLength );

    //! Prints channel connections on the console (stdout)
	void PrintConnections() const;

	// --= Realisierte Methoden der Schnittstelle "ITADatasourceRealizationEventHandler" =--

	void HandlePreGetBlockPointer( ITADatasourceRealization* pSender, unsigned int uiChannel );
	void HandlePostIncrementBlockPointer( ITADatasourceRealization* pSender);
	void HandleProcessStream( ITADatasourceRealization* pSender, const ITAStreamInfo* pStreamInfo );

private:

	//! Input description
	class InputDesc 
	{
	public:
		int iChannels;		        //!< Number of channels;
		ITAAtomicFloat fCurrentGain,fNewGain;	//!< Gain (amplification factor)
		ITAAtomicBool bMuted;					//!< Muted?
		ITADatasource* pDatasource;				//!< Datasource assigned to the input
		std::vector< const float* > vpfInputData;	//!< Pointers to the next stream blocks

		InputDesc( const int iChannels, const int iBlockLength )
		: vpfInputData( iChannels, NULL )
		{
			this->iChannels = iChannels;
			fCurrentGain = 1.0F, fNewGain = 1.0F;
			bMuted = false;
			pDatasource = NULL;
		}
	};

	//! Connections from an output channel to an input channel
	class Connection
	{
	public:
		int iFirst;								//!< Input index
		int iSecond;							//!< Input channel
		ITAAtomicFloat fCurrentGain, fNewGain;	// Gain (amplification factor)

		Connection( const int iFirst, const int iSecond )
		{
			this->iFirst = iFirst;
			this->iSecond = iSecond;
			this->fCurrentGain = 1.0F;
			this->fNewGain = 1.0F;
		}

		Connection( const int iFirst, const int iSecond, const float fGain )
		{
			this->iFirst = iFirst;
			this->iSecond = iSecond;
			this->fCurrentGain = fGain;
			this->fNewGain = fGain;
		}
	};

	struct CompareConnection
	{
		bool operator()( const Connection &lhs, const Connection &rhs ) const
		{
			// Important: Define a linear ordering relation for set! The order is unimportant.
			if( lhs.iFirst == rhs.iFirst )
				return( lhs.iSecond < rhs.iSecond );
			else
				return( lhs.iFirst < rhs.iFirst );
		}
	};

	typedef std::vector< std::set< Connection, CompareConnection > > Connections;

	//! Output description
	class OutputDesc : public ITADatasourceRealization
	{
	public:
		int iChannels;				// Number of channels;
		ITAAtomicFloat fCurrentGain, fNewGain;	// Gain (amplification factor)
		ITAAtomicBool bMuted;					// Muted?
		Connections conns;						// Input -> Output assignments

		OutputDesc( ITAStreamPatchbay* pParent,
			        const int iChannels,
				    const double dSamplerate,
			        const int iBlockLength ) 
		: ITADatasourceRealization( (unsigned int) iChannels, dSamplerate, iBlockLength )
		{
			this->iChannels = iChannels;
			fCurrentGain = 1.0F,fNewGain = 1.0F;
			bMuted = false;
			SetStreamEventHandler( pParent );
			conns.resize( iChannels );
		};

		~OutputDesc() {};
	};

	double m_dSamplerate;
	int m_iBlockLength;
	int m_iGainAdaption;
	int m_iGainFadeLength;
	std::vector< InputDesc* > m_vpInputs;
	std::vector< OutputDesc* > m_vpOutputs;
	ITAAtomicBool m_bProcessData;
	ITAAtomicBool m_bProcessIncrement;
	float m_fTempGain;

	//! Processes all the data for one streaming cycle
	void ProcessData( const ITAStreamInfo* pStreamInfo );
};

#endif // INCLUDE_WATCHER_ITA_STREAM_PATCH_BAY
