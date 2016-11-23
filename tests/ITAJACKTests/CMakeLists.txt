cmake_minimum_required( VERSION 2.8 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITADataSourcesTest )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

add_definitions( -DITA_DATA_SOURCES_DLL )


add_executable( ITAJackLoopback ITAJackLoopback.cpp )
target_link_libraries( ITAJackLoopback ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITAJackLoopback )
vista_install( ITAJackLoopback )
vista_create_default_info_file( ITAJackLoopback )

set_property( TARGET ITAJackLoopback PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )

