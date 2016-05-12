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
// $Id: ITAStreamProbe.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_PROBE
#define INCLUDE_WATCHER_ITA_STREAM_PROBE

#include <ITADataSourcesDefinitions.h>
#include <ITADataSource.h>

#include <ITAAtomicPrimitives.h>
#include <ITAAudiofileCommon.h>
#include <string>
#include <vector>

// Vorwärtedeklarationen
class ITAAudiofileWriter;

//! Eine "Messspitze mit Lautsprecher" für Audiostreams
/**
 * Instanzen der Klasse ITAStreamProbe können als "Messspitze" in Audiostreams
 * eingehängt werden. Sie schreiben dann alle vorbeigeflossenen Daten in eine Audiodatei.
 */
class ITA_DATA_SOURCES_API ITAStreamProbe : public ITADatasource {
public:
	//! Konstruktor
	/**
	 * \param pDatasource Datenquelle
	 *
	 * \note Es darf kein Nullzeiger übergeben werden.
	 */
	ITAStreamProbe(ITADatasource* pDatasource, const std::string& sFilename, ITAQuantization iQuantization=ITA_INT16);	

	//! Destruktor
	virtual ~ITAStreamProbe();

	//! Datenquelle zurückgeben
	ITADatasource* GetDatasource() const { return m_pDatasource; }

	//! Dateiname zurückgeben
	std::string GetFilename() const { return m_sFilename; }

	//! Anzahl der aufgenommenen Samples zurückgeben
	unsigned int GetNumRecordedSamples() const { return m_iRecordedSamples; }

	// -= Überladene Methoden von ITADatasource =-

	unsigned int GetBlocklength() const;
	unsigned int GetNumberOfChannels() const;
	double GetSamplerate() const;

	virtual const float* GetBlockPointer(unsigned int uiChannel, const ITAStreamInfo* pStreamInfo);	
	virtual void IncrementBlockPointer();

protected:
	ITADatasource* m_pDatasource;		//!< Angeschlossene Datenquelle
	std::string m_sFilename;			//!< Dateiname
	double m_dSamplerate;				//!< Abtastrate [Hz]
	unsigned int m_uiChannels;			//!< Anzahl Kanäle
	unsigned int m_uiBlocklength;		//!< Streaming Puffergröße [Samples]
	ITAAtomicInt m_iRecordedSamples;	//!< Anzahl aufgenommene Samples
	ITAAudiofileWriter* m_pWriter;
	float* m_pfBuffer;
	std::vector<float*> m_vpfAlias;
	bool* m_pbDataPresent;

};

#endif // INCLUDE_WATCHER_ITA_STREAM_PROBE
