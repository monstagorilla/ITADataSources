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
// $Id: ITAFileDataSource.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_FILE_DATA_SOURCE
#define INCLUDE_WATCHER_ITA_FILE_DATA_SOURCE

#include <ITADataSourcesDefinitions.h>
          
#include "ITABufferDataSource.h"
              
#include <string>		// Strings der Standard Template Library (STL)
#include <vector>

//! Audiodatei-Datenquelle
/**
 * Die Klasse ITAFileDatasource realisiert eine Datenquelle, welche
 * die Audiodaten aus einer Audiodatei freisetzt. Bei der Erzeugung
 * von Instanzen der Klasse l�dt der Konstruktor die Audiodatei
 * vollst�ndig in den Speicher. Die Anzahl der Samples in der Audiodatei -
 * auch Kapazit�t der Audiodatei genannt - wird dabei, durch das Anh�ngen
 * von Nullsamples, auf ein ganzzahliges Vielfaches der Blockl�nge erg�nzt.
 * So ergibt sich eine Kapazit�t der Datenquelle, d.h. die Anzahl an Samples
 * die die Datenquelle freisetzt. Diese kann durch das ggf. Anh�ngen der
 * Nullsamples ein anderer Wert sein als die Kapazit�t der Eingabedatei.
 * 
 * Ist das Ende der Audiodaten erreicht, gibt die Datenquelle keine Daten
 * mehr zur�ck (Nullzeiger). Im Wiederholungsmodus springt die
 * Datenquelle nach der Freisetzung des letzten Blocks Audiodaten
 * wieder zur�ck an den Anfang (Hinweis: Aufgrund der Erweiterung
 * der Audiodaten auf ein Vielfaches der Blockl�nge finden Sie das
 * erste Sample der Audiodaten stets am Offset 0 des entsprechenden
 * (wiederholten) Blockes. Die Datenquelle kann manuell zur�ckgespult werden.
 *
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITAFileDatasource : public ITABufferDatasource {
public:
	//! Konstruktor
	/**
	 * \param sFilename	Dateiname der Audiodatei
	 * \param uiBlocklength Blockl�nge
	 * \param bLoopMode Wiederholungsmodus (Optional)
	 *
	 * \note Bei Fehlern werden ISPL::Exceptions aufgel�st.
	 */
	ITAFileDatasource(std::string sFilename,
		              unsigned int uiBlocklength,
				      bool bLoopMode=false);	

	//! Destruktor
	virtual ~ITAFileDatasource();

	//! Dateinamen der Audiodatei zur�ckgeben
	std::string GetFileName() const;

	//! Kapazit�t der Eingabedatei zur�ckgeben
	/** 
	 * Gibt die Anzahl der Samples in der Eingabedatei zur�ck.
	 */
	unsigned int GetFileCapacity() const;

private:	
	std::string m_sFilename;			// Dateiname der Audiodatei
	unsigned int m_uiFileCapacity;		// Kapazit�t des Eingabedatei
	std::vector<float*> vpfData;		// Datenpuffer (Kanalweise)
};

#endif // INCLUDE_WATCHER_ITA_FILE_DATA_SOURCE
