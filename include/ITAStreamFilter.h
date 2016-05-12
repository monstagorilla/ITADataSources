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
// $Id: ITAStreamFunctionGenerator.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_FILTER
#define INCLUDE_WATCHER_ITA_STREAM_FILTER

#include <ITADataSourcesDefinitions.h>

#include <ITADataSource.h>
#include <ITATypes.h>

#include <vector>

class ITAStreamFilter : public ITADatasource
{
public:
	//! Konstruktor
	ITAStreamFilter(ITADatasource* pdsInput=NULL);	

	//! Destruktor
	virtual ~ITAStreamFilter();

	//! Eingangsdatenquelle zurückgeben
	ITADatasource* GetInputDatasource() const;

	// TODO: Erstmal aus...
	//! Eingangsdatenquelle setzen
	//void SetInputDatasource(ITADatasource* pdsInput);
	
	// -= Realisierung der abstrakten Methoden von "ITADatasource" =-
    const float* GetBlockPointer(unsigned int uiChannel, bool bWaitForData=true);
	void IncrementBlockPointer();

protected:
	//! Verarbeitungsmethode
	/**
	 * In dieser Methode werden die neue Eingangsdaten aller Kanäle verarbeitet und das Filter berechnet.
	 */
	virtual void ProcessAllChannels(const float** ppfInputData, float** ppfOutputData) {
		/* 
		   Hinweis: Diese Vorgabe-Implementierung setzt den Methodenaufruf in die kanalweise
		            Verarbeitung mittels der Methode "ProcessChannel" um. Diese Verfahrensweise
					ist adäquat für Filter welche eine kanal-unabhängige Verarbeitung erlauben.

					Möchten Sie einen Filter mit kanal-abhängiger Verarbeitung bauen,
					so muss diese Methode entsprechend angepasst werden.

					Hierbei müssen Sie nicht zwangsläufig die Methode "ProcessChannel" involvieren.
	    */

		for (unsigned int i=0; i<m_uiChannels; i++) ProcessChannel(i, ppfInputData[i], ppfOutputData[i]);
	}

	//! Verarbeitungsmethode
	/**
	 * In dieser Methode werden für einen Kanal neue Eingangsdaten verarbeitet und das Filter berechnet.
	 * Diese Methode wird die kanal-unabhängige Datenverarbeitung des Filters realisiert. Kanal-unabhängig
	 * bedeutet, dass die Ausgangsdaten dieses Kanals einzig aus den Eingangsdaten des selben Kanals 
	 * berechnet werden können.
	 */
	virtual void ProcessChannel(unsigned int uiChannel, const float* pfInputData, float* pfOutputData) {
		// Dies ist eine Vorgabe-Implementierung, welche die Eingangsdaten unverändert in die Ausgabe kopiert
		//fm_copy(pfOutputData, pfInputData, m_uiBlocklength);
		memcpy(pfOutputData, pfInputData, m_uiBlocklength * sizeof(float));
	}

private:
	ITADatasource* m_pdsInput;					// Eingangsdatenquelle
	std::vector<const float*> m_vpfInputData;	// Eingangsdatenzeiger
	std::vector<float*> m_vpfOutputBuffer;		// Ausgabepuffer
	bool m_bGBPCalled;							// Flag die Anzeigt ob seit dem letzten IncrementBlockPointer (IBP)
												// bereits ein erneuter Eintritt in GetBlockPointer (GBP) erfolgt ist
};

#endif // INCLUDE_WATCHER_ITA_STREAM_FILTER