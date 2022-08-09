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

#ifndef INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER
#define INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER

#include <ITADataSourceRealization.h>
#include <ITADataSourcesDefinitions.h>
#include <ITATypes.h>

class ITADatasourceRealization;

//! 1:n-Verzweigung für Audiodatenströme
/**
 * Die Klasse ITAStreamAmplifier realisiert eine 1:n-Verzweigung für Audiodatenströme.
 * Hierbei wird eine Eingangsdatenquelle auf n Ausgänge repliziert.
 */
class ITA_DATA_SOURCES_API ITAStreamAmplifier : public ITADatasourceRealization
{
public:
	ITAStreamAmplifier( ITADatasource* pdsInput = NULL, float fInitialGain = 1.0F );
	virtual ~ITAStreamAmplifier( );

	//! An den Eingang angeschlossene Datenquelle zurückgeben
	ITADatasource* GetInputDatasource( ) const;

	//! Gibt zurück ob die Stummschaltung eingeschaltet ist
	bool IsMuted( ) const;

	//! Stummschaltung ein-/ausschalten
	void SetMuted( bool bMuted );

	//! Verstärkung [Faktor] zurückgeben
	float GetGain( ) const;

	//! Verstärkung setzen
	void SetGain( float fGain );

	// -= Redefinierte Methoden der Klasse "ITADatasource" =-

	virtual void ProcessStream( const ITAStreamInfo* pStreamInfo );
	virtual void PostIncrementBlockPointer( );

protected:
	ITADatasource* m_pInputDatasource;
	std::atomic<bool> m_bMuted;
	std::atomic<float> m_fCurGain;
	float m_fPrevGain;
};

#endif // INCLUDE_WATCHER_ITA_STREAM_AMPLIFIER
