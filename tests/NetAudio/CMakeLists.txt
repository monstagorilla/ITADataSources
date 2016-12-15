cmake_minimum_required( VERSION 2.9 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITANetAudioTests )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( VistaCoreLibs REQUIRED COMPONENTS VistaInterProcComm FIND_DEPENDENCIES )
vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )


add_executable( ITANABufferServerTest ITANABufferServerTest.cpp )
target_link_libraries( ITANABufferServerTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANABufferServerTest )
vista_install( ITANABufferServerTest )
vista_create_default_info_file( ITANABufferServerTest )

set_property( TARGET ITANABufferServerTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )


add_executable( ITANAPlayerTest ITANAPlayerTest.cpp )
target_link_libraries( ITANAPlayerTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANAPlayerTest )
vista_install( ITANAPlayerTest )
vista_create_default_info_file( ITANAPlayerTest )

set_property( TARGET ITANAPlayerTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )


add_executable( ITANetAudioSampleServerTest ITANetAudioSampleServerTest.cpp )
target_link_libraries( ITANetAudioSampleServerTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANetAudioSampleServerTest )
vista_install( ITANetAudioSampleServerTest )
vista_create_default_info_file( ITANetAudioSampleServerTest )

set_property( TARGET ITANetAudioSampleServerTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )


add_executable( ITANetAudioClientTest ITANetAudioClientTest.cpp )
target_link_libraries( ITANetAudioClientTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANetAudioClientTest )
vista_install( ITANetAudioClientTest )
vista_create_default_info_file( ITANetAudioClientTest )

set_property( TARGET ITANetAudioClientTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )
