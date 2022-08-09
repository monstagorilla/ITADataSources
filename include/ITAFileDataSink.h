/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2022
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

#include <ITAAudiofileCommon.h>
#include <ITADataSourcesDefinitions.h>
#include <ITAStreamInfo.h>

// STL-Includes
#include <string>
#include <vector>

// Vorw�rtsdeklarationen
class ITAAudiofileWriter;
class ITADatasource;

//! Audiodatei-Datensenke
/**
 * Die Klasse ITAFileDatasink realisiert eine Datensenke f�r Audiostreams,
 * welche die Audiodaten in eine Datei schreibt. Der Datentransfer muss
 * selbst initiiert werden.
 */
class ITA_DATA_SOURCES_API ITAFileDatasink
{
public:
	//! Konstruktor
	/**
	 * \param sFilename	Dateiname der Audiodatei
	 * \param pdsSource Datenquelle
	 *
	 * \note Bei Fehlern werden ITAExceptions aufgel�st.
	 */
	ITAFileDatasink( std::string sFilename, ITADatasource* pdsSource, ITAQuantization eQuantization = ITAQuantization::ITA_FLOAT );

	//! Destruktor
	virtual ~ITAFileDatasink( );

	//! Audiodaten transferrieren
	/**
	 * Liest die gew�nschte Anzahl Samples von der angeschlossenen Datenquelle
	 * und schreibt diese in die Audiodatei. Hierbei wird die Anzahl Samples
	 * auf das n�chstgr��ere Vielfache der Blockl�nge aufgerundet.
	 */
	void Transfer( unsigned int uiSamples );

private:
	std::string m_sFilename;           // Dateiname der Audiodatei
	ITADatasource* m_pdsSource;        // Datenquelle
	ITAStreamInfo m_siState;           // Streamzustand
	ITAAudiofileWriter* m_pFileWriter; // Dateischreiber
	std::vector<float*> m_vpfData;     // Puffervektor
	float* m_pfSilence;                // Block Stille
};

#endif // INCLUDE_WATCHER_ITA_FILE_DATA_SINK