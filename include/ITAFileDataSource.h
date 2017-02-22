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
 * von Instanzen der Klasse lädt der Konstruktor die Audiodatei
 * vollständig in den Speicher. Die Anzahl der Samples in der Audiodatei -
 * auch Kapazität der Audiodatei genannt - wird dabei, durch das Anhängen
 * von Nullsamples, auf ein ganzzahliges Vielfaches der Blocklänge ergänzt.
 * So ergibt sich eine Kapazität der Datenquelle, d.h. die Anzahl an Samples
 * die die Datenquelle freisetzt. Diese kann durch das ggf. Anhängen der
 * Nullsamples ein anderer Wert sein als die Kapazität der Eingabedatei.
 * 
 * Ist das Ende der Audiodaten erreicht, gibt die Datenquelle keine Daten
 * mehr zurück (Nullzeiger). Im Wiederholungsmodus springt die
 * Datenquelle nach der Freisetzung des letzten Blocks Audiodaten
 * wieder zurück an den Anfang (Hinweis: Aufgrund der Erweiterung
 * der Audiodaten auf ein Vielfaches der Blocklänge finden Sie das
 * erste Sample der Audiodaten stets am Offset 0 des entsprechenden
 * (wiederholten) Blockes. Die Datenquelle kann manuell zurückgespult werden.
 *
 * \ingroup datasources
 */
class ITA_DATA_SOURCES_API ITAFileDatasource : public ITABufferDatasource
{
public:
	//! Konstruktor
	/**
	 * \param sFilename	Dateiname der Audiodatei
	 * \param uiBlocklength Blocklänge
	 * \param bLoopMode Wiederholungsmodus (Optional)
	 *
	 * \note Bei Fehlern werden ISPL::Exceptions aufgelöst.
	 */
	ITAFileDatasource(std::string sFilename,
		              unsigned int uiBlocklength,
				      bool bLoopMode=false);	

	//! Destruktor
	virtual ~ITAFileDatasource();

	//! Dateinamen der Audiodatei zurückgeben
	std::string GetFileName() const;

	//! Kapazität der Eingabedatei zurückgeben
	/** 
	 * Gibt die Anzahl der Samples in der Eingabedatei zurück.
	 */
	unsigned int GetFileCapacity() const;

private:	
	std::string m_sFilename;			// Dateiname der Audiodatei
	unsigned int m_uiFileCapacity;		// Kapazität des Eingabedatei
	std::vector<float*> vpfData;		// Datenpuffer (Kanalweise)
};

#endif // INCLUDE_WATCHER_ITA_FILE_DATA_SOURCE
