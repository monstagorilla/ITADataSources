cmake_minimum_required( VERSION 2.9 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITADataSourcesTest )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( VistaCoreLibs REQUIRED COMPONENTS VistaInterProcComm FIND_DEPENDENCIES )
vista_use_package( ITABase REQUIRED FIND_DEPENDENCIES )


add_executable( ITAVistaNetTest VistaNetTest.cpp )
target_link_libraries( ITAVistaNetTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITAVistaNetTest )
vista_install( ITAVistaNetTest )
vista_create_default_info_file( ITAVistaNetTest )

set_property( TARGET ITAVistaNetTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( ITAVistaNetClient VistaNetClient.cpp )
target_link_libraries( ITAVistaNetClient ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_set_target_msvc_arguments( ITAVistaNetClient "localhost 12343" )

vista_configure_app( ITAVistaNetClient )
vista_install( ITAVistaNetClient )
vista_create_default_info_file( ITAVistaNetClient )

set_property( TARGET ITAVistaNetClient PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( ITAVistaNetServer VistaNetServer.cpp )
target_link_libraries( ITAVistaNetServer ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_set_target_msvc_arguments( ITAVistaNetServer "localhost 12343" )

vista_configure_app( ITAVistaNetServer )
vista_install( ITAVistaNetServer )
vista_create_default_info_file( ITAVistaNetServer )

set_property( TARGET ITAVistaNetServer PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )

