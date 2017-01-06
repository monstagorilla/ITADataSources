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

#ifndef INCLUDE_WATCHER_ITA_FILE_DATA_SINK
#define INCLUDE_WATCHER_ITA_FILE_DATA_SINK

#include <ITADataSourcesDefinitions.h>
          
#include <ITAAudiofileCommon.h>
#include <ITAStreamInfo.h>

// STL-Includes
#include <string> 
#include <vector>

// Vorwärtsdeklarationen
class ITAAudiofileWriter;
class ITADatasource;

//! Audiodatei-Datensenke
/**
 * Die Klasse ITAFileDatasink realisiert eine Datensenke für Audiostreams,
 * welche die Audiodaten in eine Datei schreibt. Der Datentransfer muss
 * selbst initiiert werden.
 */
class ITA_DATA_SOURCES_API ITAFileDatasink {
public:
	//! Konstruktor
	/**
	 * \param sFilename	Dateiname der Audiodatei
	 * \param pdsSource Datenquelle
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgelöst.
	 */
	ITAFileDatasink( std::string sFilename,
		ITADatasource* pdsSource,
		ITAQuantization eQuantization = ITAQuantization::ITA_FLOAT );

	//! Destruktor
	virtual ~ITAFileDatasink();

	//! Audiodaten transferrieren
	/**
	 * Liest die gewünschte Anzahl Samples von der angeschlossenen Datenquelle
	 * und schreibt diese in die Audiodatei. Hierbei wird die Anzahl Samples
	 * auf das nächstgrößere Vielfache der Blocklänge aufgerundet.
	 */
	void Transfer(unsigned int uiSamples);

private:	
	std::string m_sFilename;				// Dateiname der Audiodatei
	ITADatasource* m_pdsSource;				// Datenquelle
	ITAStreamInfo m_siState;				// Streamzustand
	ITAAudiofileWriter* m_pFileWriter;		// Dateischreiber
	std::vector<float*> m_vpfData;			// Puffervektor
	float* m_pfSilence;						// Block Stille
};

#endif // INCLUDE_WATCHER_ITA_FILE_DATA_SINK