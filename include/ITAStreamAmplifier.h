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

//! 1:n-Verzweigung f�r Audiodatenstr�me
/**
 * Die Klasse ITAStreamAmplifier realisiert eine 1:n-Verzweigung f�r Audiodatenstr�me.
 * Hierbei wird eine Eingangsdatenquelle auf n Ausg�nge repliziert.
 */
class ITA_DATA_SOURCES_API ITAStreamAmplifier : public ITADatasourceRealization
{
public:
	ITAStreamAmplifier( ITADatasource* pdsInput = NULL, float fInitialGain = 1.0F );
	virtual ~ITAStreamAmplifier( );

	//! An den Eingang angeschlossene Datenquelle zur�ckgeben
	ITADatasource* GetInputDatasource( ) const;

	//! Gibt zur�ck ob die Stummschaltung eingeschaltet ist
	bool IsMuted( ) const;

	//! Stummschaltung ein-/ausschalten
	void SetMuted( bool bMuted );

	//! Verst�rkung [Faktor] zur�ckgeben
	float GetGain( ) const;

	//! Verst�rkung setzen
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
