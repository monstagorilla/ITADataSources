// $Id: $

#ifndef __ITA_STREAM_INFO_H__
#define __ITA_STREAM_INFO_H__

#include <ITATypes.h>

// Diese Datenklasse beschreibt den Zustand eines Audiostreams
class ITAStreamInfo {
public:
	// Anzahl der abgespielten Samples seit Beginn des Streamings
	uint64_t nSamples;

	// TODO: Beschreiben
	double dTimecode;

	//! Standard-Konstruktor (setzt alle Werte 0)
	ITAStreamInfo() : nSamples(0), dTimecode(0) {}

    //! Destruktor
	virtual ~ITAStreamInfo() {};
};

#endif // __ITA_STREAM_INFO_H__
