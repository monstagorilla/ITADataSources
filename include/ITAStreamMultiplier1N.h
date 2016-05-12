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
// $Id: ITAStreamMultiplier1N.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_MULTIPLIER
#define INCLUDE_WATCHER_ITA_STREAM_MULTIPLIER

#include <ITADataSourcesDefinitions.h>
          
#include <ITACriticalSection.h>
#include <ITADataSourceRealization.h>
#include <ITATypes.h>

#include <vector>

// Forward declarations
class ITADatasourceRealization;

/**
 * This class takes in a mono input and gives an N channel output.
 * This class provides functions to set the gain of each channel required.
 */
class ITA_DATA_SOURCES_API ITAStreamMultiplier1N : public ITADatasourceRealization {
public:
	//! Constructor
	/**
	 * Creates a stream multiplier.
	 *
	 * \param pdsInput			Input datasource (must be mono)
	 * \param iOutputChannels	Number of multiplied output channels
	 * \param uiFadeLength      Number of samples for fading gains
	 */
	ITAStreamMultiplier1N(ITADatasource* pdsInput, int iOutputChannels, unsigned int uiFadeLength=32);

	//! Destructor
	virtual ~ITAStreamMultiplier1N();

	//! Returns the assigned input datasource
	ITADatasource* GetInputDatasource() const;

	//! Returns the number of samples used to fade gains
	unsigned int GetFadeLength() const;

	//! Sets the number of samples for fading gains
	/**
	 * \param uiFadeLength  Number of samples for fading gains
	 *
	 * \note Maximum number of samples is the blocklength
	 */
	void SetFadeLength(unsigned int uiFadeLength);

	//! Returns wheather the output is muted
	bool IsMuted() const;

	//! Muted/unmuted the output
	void SetMuted(bool bMuted);

	//! Returns the gain of an output channel
	float GetGain(unsigned int uiChannel) const;

	//! Set the gain of an output channel
	void SetGain(unsigned int uiChannel, float fGain);

	//! Returns the gains of all output channels
	void GetGains(std::vector<float>& vfGains) const;

	//! Sets the gains of all output channels
	void SetGains(const std::vector<float>& vfGains);

	// -= Redefined methods of class "ITADatasource" =-

	void ProcessStream(const ITAStreamInfo* pStreamInfo);
	void PostIncrementBlockPointer();

protected:
	ITADatasource* m_pInputDatasource;
	bool m_bMuted;
	std::vector<float> m_vfCurrentGains,m_vfNewGains;
	unsigned int m_iOutputChannels;
	unsigned int m_uiFadeLength;
	ITACriticalSection m_cs;		// Exklusiver Zugriff auf die Parameter (Stummschaltung & Gain)
};

#endif // INCLUDE_WATCHER_ITA_STREAM_MULTIPLIER