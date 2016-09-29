cmake_minimum_required( VERSION 2.8 )

project( ITADataSources )

list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
include( VistaCommon )


# dependencies
vista_use_package( ITABase REQUIRED FIND_DEPENDENCIES )
vista_use_package( ASIO QUIET )
vista_use_package( Portaudio QUIET )

if( NOT DEFINED ITA_CORE_LIBS_BUILD_STATIC )
	set( ITA_CORE_LIBS_BUILD_STATIC OFF CACHE BOOL "Build all ITA core libs in static mode" )
endif( NOT DEFINED ITA_CORE_LIBS_BUILD_STATIC )

if( NOT DEFINED ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )
	set( ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO ${VASIO_FOUND} CACHE BOOL "Build with ASIO support" )
endif( NOT DEFINED ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )

if( NOT DEFINED ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )
	set( ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO ${VPORTAUDIO_FOUND} CACHE BOOL "Build with Portaudio support" )
endif( NOT DEFINED ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )


# includes
include_directories( "include" )


# sources
set( ITADataSourcesHeader
	"include/ITABufferDataSink.h"
	"include/ITABufferDataSource.h"
	"include/ITADataSource.h"
	"include/ITADataSourceRealization.h"
	"include/ITADataSourcesDefinitions.h"
	"include/ITAFileDataSink.h"
	"include/ITAFileDataSource.h"
	"include/ITAPeakDetector.h"
	"include/ITAStreamAmplifier.h"
	#"include/ITAStreamFilter.h"
	"include/ITAStreamFunctionGenerator.h"
	"include/ITAStreamModalSynthesizer.h"
	"include/ITAStreamMultiplier1N.h"
	"include/ITAStreamPatchBay.h"
	"include/ITAStreamProbe.h"
	"include/ITAStreamProperties.h"
	"include/ITAStreamYJunction.h"
	)

set( ITADataSourcesSources
	"src/ITABufferDataSink.cpp"
	"src/ITABufferDataSource.cpp"
	"src/ITADataSource.cpp"
	"src/ITADataSourceRealization.cpp"	
	"src/ITAFileDataSink.cpp"
	"src/ITAFileDataSource.cpp"
	"src/ITAPeakDetector.cpp"
	"src/ITAStreamAmplifier.cpp"
	#"src/ITAStreamFilter.cpp"
	"src/ITAStreamFunctionGenerator.cpp"
	"src/ITAStreamModalSynthesizer.cpp"
	"src/ITAStreamMultiplier1N.cpp"
	"src/ITAStreamPatchBay.cpp"
	"src/ITAStreamProbe.cpp"
	"src/ITAStreamProperties.cpp"	
	"src/ITAStreamYJunction.cpp"
	)

# Filter windows-only components
if( WIN32 )
	list( APPEND ITADataSourcesHeader "include/ITAStreamPump.h" "include/ITADataSourceUtils.h" )
	list( APPEND ITADataSourcesSources "src/ITAStreamPump.cpp" "src/ITADataSourceUtils.cpp" )
endif( WIN32 )

if( VASIO_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )
	list( APPEND ITADataSourcesHeader "include/ITAAsioInterface.h" )
	list( APPEND ITADataSourcesSources "src/ITAAsioInterface.cpp" )
	add_definitions( -DIEEE754_64FLOAT=1 )
endif( VASIO_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )
if( VPORTAUDIO_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )
	list( APPEND ITADataSourcesHeader "include/ITAPortaudioInterface.h" )
	list( APPEND ITADataSourcesSources "src/ITAPortaudioInterface.cpp" )
endif( VPORTAUDIO_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )


# compiler settings
if( NOT ITA_CORE_LIBS_BUILD_STATIC )
	add_definitions( -DITA_DATA_SOURCES_EXPORT )
else( NOT ITA_CORE_LIBS_BUILD_STATIC )
	add_definitions( -DITA_BASE_STATIC -DITA_DATA_SOURCES_STATIC )
endif( NOT ITA_CORE_LIBS_BUILD_STATIC )

if( NOT WIN32 )
	add_definitions( -std=c++11 )
endif( NOT WIN32)


# linker
add_library( ITADataSources SHARED ${ITADataSourcesHeader} ${ITADataSourcesSources} )
target_link_libraries( ITADataSources ${VISTA_USE_PACKAGE_LIBRARIES} )


# config
vista_configure_lib( ITADataSources )
vista_install( ITADataSources )
set( ITADATASOURCES_INCLUDE_OUTDIR "${CMAKE_CURRENT_SOURCE_DIR}/include" )
vista_create_cmake_configs( ITADataSources )
vista_create_default_info_file( ITADataSources )

set_property( TARGET ITADataSources PROPERTY FOLDER "ITACoreLibs" )

	
# apps
set( ITADATASOURCES_COMMON_BUILD TRUE )
add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/apps/ita_whad" )


# tests
set( ITADATASOURCES_COMMON_BUILD TRUE )
add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/tests" )
