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

#ifndef INCLUDE_WATCHER_ITA_STREAM_INFO
#define INCLUDE_WATCHER_ITA_STREAM_INFO

#include <ITATypes.h>

//! Time code information for audio streams
class ITAStreamInfo
{
public:
	uint64_t nSamples;      //!< Number of samples processed
	double dStreamTimeCode; //!< Stream time code (starts with zero)
	double dSysTimeCode;    //!< System time stamp code (begings with current time stamp of system)

	inline ITAStreamInfo( ) : nSamples( 0 ), dStreamTimeCode( 0.0f ), dSysTimeCode( 0.0f ) { };

	inline virtual ~ITAStreamInfo( ) { };
};

#endif // INCLUDE_WATCHER_ITA_STREAM_INFO
