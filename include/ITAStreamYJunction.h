/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2018
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

#ifndef INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION
#define INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION

#include <ITADataSourcesDefinitions.h>

#include <ITATypes.h>

class ITADatasource;
class ITAStreamPatchbay;

//! 1:n-Verzweigung f�r Audiodatenstr�me
/**
 * Die Klasse ITAStreamYJunction realisiert eine 1:n-Verzweigung f�r Audiodatenstr�me.
 * Hierbei wird eine Eingangsdatenquelle auf n Ausg�nge repliziert.
 */
class ITA_DATA_SOURCES_API ITAStreamYJunction
{
public:
	ITAStreamYJunction( unsigned int uiOutputs, ITADatasource* pdsInput = NULL );

	virtual ~ITAStreamYJunction();

	//! An den Eingang angeschlossene Datenquelle zur�ckgeben
	ITADatasource* GetInputDatasource();

	//! Eingangsdatenquelle setzen
	/**
	 * Setzt die eingangsseitige Datenquelle.
	 *
	 * \important Die Methode darf nicht mit dem Nullzeiger als Parameter aufgerufen werden.
	 *            Ferner muss die neue Datenquelle die gleichen Eigenschaften wie die zuvor
	 *            zugeordnete Datenquelle aufweisen, falls letztere zugeordnet war.
	 *
	 * \note Im Fehlerfall l�st die Methode ein ITAException aus
	 */
	void SetInputDatasource( ITADatasource* pdsInput );

	//! Anzahl der Ausg�nge zur�ckgeben
	unsigned int GetNumberOfOutputs();

	//! Eine Ausgangsdatenquelle zur�ckgeben
	ITADatasource* GetOutputDatasource( unsigned int uiIndex );

protected:
	ITAStreamPatchbay* m_pImpl;
};

#endif // INCLUDE_WATCHER_ITA_STREAM_Y_JUNCTION