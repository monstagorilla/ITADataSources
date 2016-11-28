cmake_minimum_required( VERSION 2.8 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITADataSourcesTest )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

add_definitions( -DITA_DATA_SOURCES_DLL )


add_executable( ListDevices ListDevices.cpp )
target_link_libraries( ListDevices ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ListDevices )
vista_install( ListDevices )
vista_create_default_info_file( ListDevices )

set_property( TARGET ListDevices PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( Loopback Loopback.cpp )
target_link_libraries( Loopback ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( Loopback )
vista_install( Loopback )
vista_create_default_info_file( Loopback )

set_property( TARGET Loopback PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( Play Play.cpp )
target_link_libraries( Play ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( Play )
vista_install( Play )
vista_create_default_info_file( Play )

set_property( TARGET Play PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( Record Record.cpp )
target_link_libraries( Record ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( Record )
vista_install( Record )
vista_create_default_info_file( Record )

set_property( TARGET Record PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( ThreadInit ThreadInit.cpp )
target_link_libraries( ThreadInit ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ThreadInit )
vista_install( ThreadInit )
vista_create_default_info_file( ThreadInit )

set_property( TARGET ThreadInit PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )

