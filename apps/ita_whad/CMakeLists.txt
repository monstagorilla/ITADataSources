cmake_minimum_required( VERSION 2.8 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ita_whad )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

if( ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )
	add_definitions( "-DITA_WHAD_WITH_ASIO" )
endif( ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )

if( ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )
	add_definitions( "-DITA_WHAD_WITH_PORTAUDIO" )
endif( ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )

add_definitions( -DITA_DATA_SOURCES_DLL )


add_executable( ita_whad main.cpp )
target_link_libraries( ita_whad ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ita_whad )
vista_install( ita_whad )
vista_create_default_info_file( ita_whad )

set_property( TARGET ita_whad PROPERTY FOLDER "ITACoreLibs/Apps/ITADataSources" )
