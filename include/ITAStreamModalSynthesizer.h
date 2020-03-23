/*
 * ----------------------------------------------------------------
 *
 *		ITA core libs
 *		(c) Copyright Institute of Technical Acoustics (ITA)
 *		RWTH Aachen University, Germany, 2015-2020
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

#ifndef INCLUDE_WATCHER_ITA_STREAM_MODAL_SYNTHESIZER
#define INCLUDE_WATCHER_ITA_STREAM_MODAL_SYNTHESIZER

#include <ITADataSourcesDefinitions.h>
#include <ITADataSourceRealization.h>
#include <vector>

//! Modal-Synthese-Datenquelle
/**
 * ...
 */
class ITA_DATA_SOURCES_API ITAStreamModalSynthesizer : public ITADatasourceRealization {
public:
	class ModeData {
	public:
		double dFrequency;	// Frequency [Hz]
		double dAmplitude;	// Amplitude
		double dLambda;		// Exponential dampening coefficient (0=undamped, >0 damped)
	};

	ITAStreamModalSynthesizer(double dSamplerate, unsigned int uiBlocklength);

	//! Löscht alle Moden (nächster Block = Stille)
	void Clear();

	//! Modendaten setzen (aktualisieren)
	void SetModes(const ModeData* pModeData, int iNumModes);

	//! Modendaten setzen (aktualisieren)
	void SetModes(std::vector<ModeData> vModeData);

private:	
	//! Internal mode representation
	class ModeDataInternal : public ModeData {
	public:
		double dTime;		// Mode time [s]
	};
};

#endif // INCLUDE_WATCHER_ITA_STREAM_MODAL_SYNTHESIZER
