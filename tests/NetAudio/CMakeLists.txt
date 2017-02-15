cmake_minimum_required( VERSION 2.9 )

project( ITANetAudioTests )
list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
include( VistaCommon )

vista_use_package( VistaCoreLibs REQUIRED COMPONENTS VistaInterProcComm FIND_DEPENDENCIES )
vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

add_definitions(-DIEEE754_64FLOAT)

add_executable( ITANetAudioStreamingPortaudioClientTest ITANetAudioStreamingPortaudioClientTest.cpp )
target_link_libraries( ITANetAudioStreamingPortaudioClientTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANetAudioStreamingPortaudioClientTest )
vista_install( ITANetAudioStreamingPortaudioClientTest )
vista_create_default_info_file( ITANetAudioStreamingPortaudioClientTest )

set_property( TARGET ITANetAudioStreamingPortaudioClientTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )


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


add_executable( ITANetAudioStreamingServerTest ITANetAudioStreamingServerTest.cpp )
target_link_libraries( ITANetAudioStreamingServerTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANetAudioStreamingServerTest )
vista_install( ITANetAudioStreamingServerTest )
vista_create_default_info_file( ITANetAudioStreamingServerTest )

set_property( TARGET ITANetAudioStreamingServerTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )


if( WIN32 ) # uses ASIO
	add_executable( ITANetAudioStreamingClientTest ITANetAudioStreamingClientTest.cpp )
	target_link_libraries( ITANetAudioStreamingClientTest ${VISTA_USE_PACKAGE_LIBRARIES} )

	vista_configure_app( ITANetAudioStreamingClientTest )
	vista_install( ITANetAudioStreamingClientTest )
	vista_create_default_info_file( ITANetAudioStreamingClientTest )

	set_property( TARGET ITANetAudioStreamingClientTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )
endif( )

add_executable( ITANetAudioTest ITANetAudioTest.cpp )
target_link_libraries( ITANetAudioTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ITANetAudioTest )
vista_install( ITANetAudioTest )
vista_create_default_info_file( ITANetAudioTest )

vista_install_all_dlls( bin )

set_property( TARGET ITANetAudioTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/NetAudio" )
