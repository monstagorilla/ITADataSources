cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (ITANetAudioTests)

# ######################################################################################################################

add_executable (ITANABufferServerTest ITANABufferServerTest.cpp)
target_link_libraries (ITANABufferServerTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITANABufferServerTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
set_property (TARGET ITANABufferServerTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITANABufferServerTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ITANetAudioStreamingServerTest ITANetAudioStreamingServerTest.cpp)
target_link_libraries (ITANetAudioStreamingServerTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITANetAudioStreamingServerTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
set_property (TARGET ITANetAudioStreamingServerTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITANetAudioStreamingServerTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

if (ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO)
	add_executable (ITANAPlayerTest ITANAPlayerTest.cpp)
	target_link_libraries (ITANAPlayerTest PUBLIC ITADataSources::ITADataSources)

	set_property (TARGET ITANAPlayerTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
	set_property (TARGET ITANAPlayerTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	install (TARGETS ITANAPlayerTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

	# ##################################################################################################################

	add_executable (ITANetAudioStreamingPortaudioClientTest ITANetAudioStreamingPortaudioClientTest.cpp)
	target_link_libraries (ITANetAudioStreamingPortaudioClientTest PUBLIC ITADataSources::ITADataSources)

	set_property (TARGET ITANetAudioStreamingPortaudioClientTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
	set_property (TARGET ITANetAudioStreamingPortaudioClientTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	install (TARGETS ITANetAudioStreamingPortaudioClientTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

endif ()

# ######################################################################################################################

if (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO)

	add_executable (ITANetAudioStreamingClientTest ITANetAudioStreamingClientTest.cpp)
	target_link_libraries (ITANetAudioStreamingClientTest PUBLIC ITADataSources::ITADataSources)

	set_property (TARGET ITANetAudioStreamingClientTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
	set_property (TARGET ITANetAudioStreamingClientTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	install (TARGETS ITANetAudioStreamingClientTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

endif ()

# ######################################################################################################################

if (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO AND ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO)

	add_executable (ITANetAudioTest ITANetAudioTest.cpp)
	target_link_libraries (ITANetAudioTest PUBLIC ITADataSources::ITADataSources)

	set_property (TARGET ITANetAudioTest PROPERTY FOLDER "Tests/ITADataSources/NetAudio")
	set_property (TARGET ITANetAudioTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	install (TARGETS ITANetAudioTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

endif ()

# ######################################################################################################################
