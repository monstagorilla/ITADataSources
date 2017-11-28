cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project( ITADataSources )

list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
include( VistaCommon )


# dependencies
vista_use_package( ITABase REQUIRED FIND_DEPENDENCIES )
vista_use_package( VistaCoreLibs REQUIRED COMPONENTS VistaInterProcComm FIND_DEPENDENCIES )
vista_use_package( ASIO QUIET )
vista_use_package( Portaudio QUIET )
vista_use_package( JACK QUIET )


if( NOT DEFINED ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO )
	set( ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO ${VASIO_FOUND} CACHE BOOL "Build with ASIO support" )
endif( )

if( NOT DEFINED ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO )
	set( ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO ${VPORTAUDIO_FOUND} CACHE BOOL "Build with Portaudio support" )
endif( )

if( NOT DEFINED ITA_DATA_SOURCES_WITH_INTEGRATED_JACK )
	set( ITA_DATA_SOURCES_WITH_INTEGRATED_JACK ${VJACK_FOUND} CACHE BOOL "Build with JACK2 support" )
endif( )

if( NOT DEFINED ITA_DATA_SOURCES_WITH_NET_AUDIO )
	set( ITA_DATA_SOURCES_WITH_NET_AUDIO ON CACHE BOOL "Build with NetAudio support" )
endif( )

if( NOT DEFINED ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC )
	set( ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC OFF CACHE BOOL "Show a lot of traffic information with NetAudio (debugging only)" )
	mark_as_advanced( ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC )
endif( )

if( ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC )
	add_definitions( -DNET_AUDIO_SHOW_TRAFFIC )
endif( )


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
	"include/ITAStreamDetector.h"
	"include/ITAStreamAmplifier.h"
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
	"src/ITAStreamDetector.cpp"
	"src/ITAStreamAmplifier.cpp"
	"src/ITAStreamFunctionGenerator.cpp"
	"src/ITAStreamModalSynthesizer.cpp"
	"src/ITAStreamMultiplier1N.cpp"
	"src/ITAStreamPatchBay.cpp"
	"src/ITAStreamProbe.cpp"
	"src/ITAStreamProperties.cpp"	
	"src/ITAStreamYJunction.cpp"
	)

list( APPEND ITADataSourcesSources "src/ITADataSourceUtils.cpp" )

# Filter windows-only components
if( WIN32 )
	list( APPEND ITADataSourcesHeader "include/ITAStreamPump.h" "include/ITADataSourceUtils.h" )
	list( APPEND ITADataSourcesSources "src/ITAStreamPump.cpp" "src/ITADataSourceUtils.cpp" )
endif( )

if( VASIO_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO )
	list( APPEND ITADataSourcesHeader "include/ITAAsioInterface.h" )
	list( APPEND ITADataSourcesSources "src/ITAAsioInterface.cpp" )
	add_definitions( -DIEEE754_64FLOAT=1 )
endif( )

if( VPORTAUDIO_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO )
	list( APPEND ITADataSourcesHeader "include/ITAPortaudioInterface.h" )
	list( APPEND ITADataSourcesSources "src/ITAPortaudioInterface.cpp" )
endif( )

if( VJACK_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_JACK )
	list( APPEND ITADataSourcesHeader "include/ITAJACKInterface.h" )
	list( APPEND ITADataSourcesSources "src/ITAJACKInterface.cpp" )
endif( )

if( ITA_DATA_SOURCES_WITH_NET_AUDIO )
	list( APPEND ITADataSourcesHeader
		"include/ITANetAudioStream.h"
		"include/ITANetAudioSampleServer.h"
		"include/ITANetAudioStreamingServer.h"
		)
	list( APPEND ITADataSourcesSources
		"src/ITANetAudioClient.cpp"
		"src/ITANetAudioClient.h"
		"src/ITANetAudioMessage.cpp"
		"src/ITANetAudioMessage.h"
		"src/ITANetAudioProtocol.h"
		"src/ITANetAudioServer.cpp"
		"src/ITANetAudioServer.h"
		"src/ITANetAudioStream.cpp"
		"src/ITANetAudioStreamingClient.cpp"
		"src/ITANetAudioStreamingClient.h"
		"src/ITANetAudioStreamingServer.cpp"
		)	
endif( )

# compiler settings
if( ITA_VISTA_BUILD_STATIC )
	add_definitions( -DVISTABASE_STATIC -DVISTAMATH_STATIC -DVISTAASPECTS_STATIC -DVISTATOOLS_STATIC -DVISTAINTERPROCCOMM_STATIC )
endif( )

if( BUILD_SHARED_LIBS )
	add_definitions( -DITA_DATA_SOURCES_EXPORT )
else( )
	add_definitions( -DITA_DATA_SOURCES_STATIC )
endif( )

if( ITA_CORE_LIBS_BUILD_STATIC )
	add_definitions( -DITA_BASE_STATIC )
endif( )


#target_compile_features(ITADataSources PRIVATE cxx_range_for)
if( NOT WIN32 )
	add_definitions( -std=c++11 )
endif( )


# linker
add_library( ITADataSources ${ITADataSourcesHeader} ${ITADataSourcesSources} )
target_link_libraries( ITADataSources ${VISTA_USE_PACKAGE_LIBRARIES} )


# config
vista_configure_lib( ITADataSources )
vista_install( ITADataSources )
set( ITADATASOURCES_INCLUDE_OUTDIR "${CMAKE_CURRENT_SOURCE_DIR}/include" )
vista_create_cmake_configs( ITADataSources )
vista_create_default_info_file( ITADataSources )

set_property( TARGET ITADataSources PROPERTY FOLDER "ITACoreLibs" )

	
# apps
if( ITA_CORE_LIBS_WITH_APPS )
	set( ITADATASOURCES_COMMON_BUILD TRUE )
	add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/apps/ita_whad" )
endif( )


# tests
if( ITA_CORE_LIBS_WITH_TESTS )
	set( ITADATASOURCES_COMMON_BUILD TRUE )
	add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/tests" )
endif( )
