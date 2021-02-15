/*
* ----------------------------------------------------------------
*
*		ITA core libs
*		(c) Copyright Institute of Technical Acoustics (ITA)
*		RWTH Aachen University, Germany, 2015-2021
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

#ifndef INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS
#define INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS

#if ( defined WIN32 ) && !( defined ITA_DATA_SOURCES_STATIC )
 #ifdef ITA_DATA_SOURCES_EXPORT
  #define ITA_DATA_SOURCES_API __declspec( dllexport )
 #else
  #define ITA_DATA_SOURCES_API __declspec( dllimport )
 #endif
#else
 #define ITA_DATA_SOURCES_API
#endif

#endif // INCLUDE_WATCHER_ITA_DATA_SOURCES_DEFINITIONS
