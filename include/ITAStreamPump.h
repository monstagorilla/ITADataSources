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

#ifndef INCLUDE_WATCHER_ITA_STREAM_PUMP
#define INCLUDE_WATCHER_ITA_STREAM_PUMP

#include <ITADataSourcesDefinitions.h>

#include <ITAStreamInfo.h>
#include <ITATimer.h>

class ITADatasource;

//! Eine künstliche Pumpe für Audiostreams 
/**
 * Mit dieser Klasse kann man synchron einen Stream abgreifen lassen
 * und somit echtes asynchrones Laufzeitverhalten wie mit einer echten
 * Soundkarte nachbilden - zum Beispiel zum Debuggen.
 */
class ITA_DATA_SOURCES_API ITAStreamPump : public ITATimerEventHandler {
public:
	//! Konstruktor
	/**
	 * \param pDatasource Datenquelle
	 *
	 * \note Es darf kein Nullzeiger übergeben werden.
	 */
	ITAStreamPump(ITADatasource* pDatasource);	

	//! Destruktor
	virtual ~ITAStreamPump();

	//! Streaming beginnen
	void StartStreaming();

	//! Streaming beenden
	void StopStreaming();

	// Timer event handler
	void handleTimerEvent(const ITATimer& tSource);

protected:
	ITADatasource* m_pDatasource;	// Angeschlossene Datenquelle
	ITATimer* m_pTimer;
	unsigned int m_uiChannels;
	ITAStreamInfo m_siState;		// Streamzustand
};

#endif // INCLUDE_WATCHER_ITA_STREAM_PUMP
