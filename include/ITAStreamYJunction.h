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
// $Id: ITAStreamYJunction.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION
#define INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION

#include <ITADataSourcesDefinitions.h>
          
#include <ITATypes.h>

class ITADatasource;
class ITAStreamPatchbay;

//! 1:n-Verzweigung für Audiodatenströme
/**
 * Die Klasse ITAStreamYJunction realisiert eine 1:n-Verzweigung für Audiodatenströme.
 * Hierbei wird eine Eingangsdatenquelle auf n Ausgänge repliziert.
 */
class ITAStreamYJunction {
public:
	ITAStreamYJunction(unsigned int uiOutputs, ITADatasource* pdsInput=NULL);

	virtual ~ITAStreamYJunction();

	//! An den Eingang angeschlossene Datenquelle zurückgeben
	ITADatasource* GetInputDatasource();

	//! Eingangsdatenquelle setzen
	/**
	 * Setzt die eingangsseitige Datenquelle.
	 * 
	 * \important Die Methode darf nicht mit dem Nullzeiger als Parameter aufgerufen werden.
	 *            Ferner muss die neue Datenquelle die gleichen Eigenschaften wie die zuvor
	 *            zugeordnete Datenquelle aufweisen, falls letztere zugeordnet war.
	 *
	 * \note Im Fehlerfall löst die Methode ein ITAException aus
	 */
	void SetInputDatasource(ITADatasource* pdsInput);

	//! Anzahl der Ausgänge zurückgeben
	unsigned int GetNumberOfOutputs();

	//! Eine Ausgangsdatenquelle zurückgeben
	ITADatasource* GetOutputDatasource(unsigned int uiIndex);

protected:
	ITAStreamPatchbay* m_pImpl;
};

#endif // INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION