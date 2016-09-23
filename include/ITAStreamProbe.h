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

#ifndef INCLUDE_WATCHER_ITA_STREAM_PROBE
#define INCLUDE_WATCHER_ITA_STREAM_PROBE

#include <ITADataSourcesDefinitions.h>
#include <ITADataSource.h>

#include <ITAAtomicPrimitives.h>
#include <ITAAudiofileCommon.h>
#include <string>
#include <vector>

// Vorw�rtedeklarationen
class ITAAudiofileWriter;

//! Eine "Messspitze mit Lautsprecher" f�r Audiostreams
/**
 * Instanzen der Klasse ITAStreamProbe k�nnen als "Messspitze" in Audiostreams
 * eingeh�ngt werden. Sie schreiben dann alle vorbeigeflossenen Daten in eine Audiodatei.
 */
class ITA_DATA_SOURCES_API ITAStreamProbe : public ITADatasource
{
public:
	//! Konstruktor
	/**
	 * \param pDatasource Datenquelle
	 *
	 * \note Es darf kein Nullzeiger �bergeben werden.
	 */
	ITAStreamProbe( ITADatasource* pDatasource, const std::string& sFilename, ITAQuantization iQuantization = ITAQuantization::ITA_FLOAT );

	//! Destruktor
	virtual ~ITAStreamProbe();

	//! Datenquelle zur�ckgeben
	ITADatasource* GetDatasource() const { return m_pDatasource; }

	//! Dateiname zur�ckgeben
	std::string GetFilename() const { return m_sFilename; }

	//! Anzahl der aufgenommenen Samples zur�ckgeben
	unsigned int GetNumRecordedSamples() const { return m_iRecordedSamples; }

	// -= �berladene Methoden von ITADatasource =-

	unsigned int GetBlocklength() const;
	unsigned int GetNumberOfChannels() const;
	double GetSampleRate() const;

	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);	
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDatasource;		//!< Angeschlossene Datenquelle
	std::string m_sFilename;			//!< Dateiname
	double m_dSamplerate;				//!< Abtastrate [Hz]
	unsigned int m_uiChannels;			//!< Anzahl Kan�le
	unsigned int m_uiBlocklength;		//!< Streaming Puffergr��e [Samples]
	ITAAtomicInt m_iRecordedSamples;	//!< Anzahl aufgenommene Samples
	ITAAudiofileWriter* m_pWriter;
	float* m_pfBuffer;
	std::vector<float*> m_vpfAlias;
	bool* m_pbDataPresent;

};

#endif // INCLUDE_WATCHER_ITA_STREAM_PROBE
