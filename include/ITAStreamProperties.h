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

#ifndef INCLUDE_WATCHER_ITA_STREAM_PROPERTIES
#define INCLUDE_WATCHER_ITA_STREAM_PROPERTIES

#include <ITADataSourcesDefinitions.h>

// Diese Datenklasse beschreibt die Eigenschaften eines Audiostreams
class ITA_DATA_SOURCES_API ITAStreamProperties
{
public:
	double dSamplerate;			// Abtastrate [Hertz]
	unsigned int uiChannels;	// Anzahl Kanäle
	unsigned int uiBlocklength;	// Streaming-Blocklänge [Samples]

	//! Standard-Konstruktor (setzt alle Werte 0)
	ITAStreamProperties();

	//! Initialisierungs-Konstruktor
	ITAStreamProperties(double dSamplerate, unsigned int uiChannels, unsigned int uiBlocklength);

    //! Destruktor
	virtual ~ITAStreamProperties();

	//! Vergleichsoperatoren
	/**
	 * Gleichheit besteht, falls alle Felder (Abtastrate, Anzahl Kanäle, Blocklänge) identisch sind.
	 */
	bool operator==(const ITAStreamProperties& rhs) const;
	bool operator!=(const ITAStreamProperties& rhs) const;
};

#endif // INCLUDE_WATCHER_ITA_STREAM_PROPERTIES
