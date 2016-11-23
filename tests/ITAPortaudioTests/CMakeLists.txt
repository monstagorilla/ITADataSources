cmake_minimum_required( VERSION 2.8 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITADataSourcesTest )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

add_definitions( -DITA_DATA_SOURCES_DLL )


add_executable( ITAPortaudioInterfaceRecorder ITAPortaudioInterfaceRecorder.cpp )
target_link_libraries( ITAPortaudioInterfaceRecorder ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITAPortaudioInterfaceRecorder )
vista_install( ITAPortaudioInterfaceRecorder )
vista_create_default_info_file( ITAPortaudioInterfaceRecorder )

set_property( TARGET ITAPortaudioInterfaceRecorder PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( ITAPortaudioInterfaceTest ITAPortaudioInterfaceTest.cpp )
target_link_libraries( ITAPortaudioInterfaceTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITAPortaudioInterfaceTest )
vista_install( ITAPortaudioInterfaceTest )
vista_create_default_info_file( ITAPortaudioInterfaceTest )

set_property( TARGET ITAPortaudioInterfaceTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( ITAPortaudioMeasurementTest ITAPortaudioMeasurementTest.cpp )
target_link_libraries( ITAPortaudioMeasurementTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_set_target_msvc_arguments( ITAPortaudioMeasurementTest "${CMAKE_CURRENT_SOURCE_DIR}/ITAPortaudioMeasurement_ExcitationSignal.wav ${CMAKE_CURRENT_SOURCE_DIR}/ITAPortaudioMeasurement_Record.wav 44100 512" )

vista_configure_app( ITAPortaudioMeasurementTest )
vista_install( ITAPortaudioMeasurementTest )
vista_create_default_info_file( ITAPortaudioMeasurementTest )

set_property( TARGET ITAPortaudioMeasurementTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )
