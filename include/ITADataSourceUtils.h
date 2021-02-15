/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2021
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

#ifndef INCLUDE_WATCHER_ITA_DATA_SOURCES_UTILS
#define INCLUDE_WATCHER_ITA_DATA_SOURCES_UTILS

#include <ITADataSourcesDefinitions.h>

#include <string> // Strings der Standard Template Library (STL)

// Vorwärtsdeklarationen
class ITADatasource;

/**
 * \defgroup utilfuncs Hilfsfunktionen
 */
/*@{*/

//! Daten einer Datenquelle in einen Puffer schreiben
/**
 * Holt eine gewissen Anzahl Samples von einer Datenquelle und
 * schreibt diese in eine Puffer.
 *
 * \param pSource Zeiger auf die Datenquelle
 * \param ppfDest Zeiger auf das Array der Puffer (jeweils für die Kanäle)
 * \param uiNumberOfSamples Anzahl der Samples
 * \param dGain Verstärkungsfaktor (optional)
 * \param bOnline Echtzeit-Modus verwenden? (d.h. reale Dauern zwischen den
 *                Datenanforderungen verwenden). Falls false, werden die
 *                Daten direkt hintereinander angefordert (Maximaler Datendurchsatz)
 * \param bDisplayProgress Fortschritt auf der Konsole ausgeben? (Optional, Standard: Nein)
 *
 * \note Gibt die Datenquelle den Nullzeiger zurück, wird für
 *       den betreffenden Block Stille in den Puffer geschrieben
 * \note Ausnahmebehandlung mittels der Klasse ITAException
 */
ITA_DATA_SOURCES_API void WriteFromDatasourceToBuffer( ITADatasource* pSource, float** ppfDest, unsigned int uiNumberOfSamples, double dGain = 1.0, bool bOnline = true, bool bDisplayProgress = false );

//! Daten einer Datenquelle in eine Datei schreiben
/**
 * Holt eine gewissen Anzahl Samples von einer Datenquelle und
 * schreibt diese in eine Audiodatei.
 *
 * \param pSource Zeiger auf die Datenquelle
 * \param sFilename	Dateiname der Zieldatei
 * \param uiNumberOfSamples Anzahl der Samples
 * \param dGain Verstärkungsfaktor (optional)
 * \param bOnline Echtzeit-Modus verwenden? (d.h. reale Dauern zwischen den
 *                Datenanforderungen verwenden). Falls false, werden die
 *                Daten direkt hintereinander angefordert (Maximaler Datendurchsatz)
 * \param bDisplayProgress Fortschritt auf der Konsole ausgeben? (Optional, Standard: Nein)
 *
 * \note Gibt die Datenquelle den Nullzeiger zurück, wird für
 *       den betreffenden Block Stille in die Datei geschrieben
 * \note Ausnahmebehandlung mittels der Klasse ITAException
 */
ITA_DATA_SOURCES_API void WriteFromDatasourceToFile( ITADatasource* pSource, std::string sFilename, unsigned int uiNumberOfSamples, double dGain = 1.0, bool bOnline = true, bool bDisplayProgress = false );

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCES_UTILS
