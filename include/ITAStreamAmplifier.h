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
// $Id: ITAStreamAmplifier.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER
#define INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER

#include <ITADataSourcesDefinitions.h>
          
#include <ITAAtomicPrimitives.h>
#include <ITADataSourceRealization.h>
#include <ITATypes.h>

class ITADatasourceRealization;

//! 1:n-Verzweigung für Audiodatenströme
/**
 * Die Klasse ITAStreamAmplifier realisiert eine 1:n-Verzweigung für Audiodatenströme.
 * Hierbei wird eine Eingangsdatenquelle auf n Ausgänge repliziert.
 */
class ITA_DATA_SOURCES_API ITAStreamAmplifier : public ITADatasourceRealization {
public:
	ITAStreamAmplifier(ITADatasource* pdsInput=NULL, float fInitialGain=1.0F);
	virtual ~ITAStreamAmplifier();

	//! An den Eingang angeschlossene Datenquelle zurückgeben
	ITADatasource* GetInputDatasource() const;

	//! Gibt zurück ob die Stummschaltung eingeschaltet ist
	bool IsMuted() const;

	//! Stummschaltung ein-/ausschalten
	void SetMuted(bool bMuted);

	//! Verstärkung [Faktor] zurückgeben
	float GetGain() const;

	//! Verstärkung setzen
	void SetGain(float fGain);

	// -= Redefinierte Methoden der Klasse "ITADatasource" =-

	virtual void ProcessStream(const ITAStreamInfo* pStreamInfo);
	virtual void PostIncrementBlockPointer();

protected:
	ITADatasource* m_pInputDatasource;
	ITAAtomicBool m_bMuted;
	ITAAtomicFloat m_fCurGain;
	float m_fPrevGain;
};

#endif // INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER
