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
// $Id: ITADataSourceDefinitions.h 2900 2012-09-17 08:42:42Z stienen $

#ifndef INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS
#define INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS

#ifdef ITA_DATA_SOURCES_DLL
	#ifdef ITA_DATA_SOURCES_EXPORT
		#define ITA_DATA_SOURCES_API __declspec(dllexport)
	#else
		#define ITA_DATA_SOURCES_API __declspec(dllimport)
	#endif
#else
	#define ITA_DATA_SOURCES_API
#endif

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS
