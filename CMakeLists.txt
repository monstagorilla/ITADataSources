cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (
	ITADataSources
	VERSION 2021.0
	LANGUAGES CXX C
)

set (CMAKE_DEBUG_POSTFIX "-d")

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND WIN32)
	set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/bin)
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/lib)
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/lib)
endif ()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_property (GLOBAL PROPERTY USE_FOLDERS ON)
endif ()

add_subdirectory (external_libs)

include (CMakeDependentOption)

cmake_dependent_option (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO "Build with ASIO support" ON "TARGET asio::asio" OFF)
cmake_dependent_option (
	ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO "Build with Portaudio support" ON "TARGET portaudio" OFF
)
option (ITA_DATA_SOURCES_WITH_INTEGRATED_JACK
		"Build with JACK2 support (May require some addition to this CMake build)" OFF
)
option (ITA_DATA_SOURCES_WITH_NET_AUDIO "Build with NetAudio support" ON)
option (ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC "Show a lot of traffic information with NetAudio (debugging only)" OFF)
mark_as_advanced (ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC)
option (ITA_DATA_SOURCES_WITH_APPS "Build the applications of the library" OFF)
option (ITA_DATA_SOURCES_WITH_TESTS "Build the tests for the library" OFF)

add_library (
	${PROJECT_NAME}
	include/ITABufferDataSink.h
	include/ITABufferDataSource.h
	include/ITADataSource.h
	include/ITADataSourceRealization.h
	include/ITADataSourcesDefinitions.h
	include/ITADataSourceDelegator.h
	include/ITAFileDataSink.h
	include/ITAFileDataSource.h
	include/ITAStreamDetector.h
	include/ITAStreamAmplifier.h
	include/ITAStreamFunctionGenerator.h
	include/ITAStreamModalSynthesizer.h
	include/ITAStreamMultiplier1N.h
	include/ITAStreamPatchBay.h
	include/ITAStreamProbe.h
	include/ITAStreamProperties.h
	include/ITAStreamYJunction.h
	include/ITAStreamInfo.h
	src/ITABufferDataSink.cpp
	src/ITABufferDataSource.cpp
	src/ITADataSource.cpp
	src/ITADataSourceRealization.cpp
	src/ITAFileDataSink.cpp
	src/ITAFileDataSource.cpp
	src/ITAStreamDetector.cpp
	src/ITAStreamAmplifier.cpp
	src/ITAStreamFunctionGenerator.cpp
	src/ITAStreamModalSynthesizer.cpp
	src/ITAStreamMultiplier1N.cpp
	src/ITAStreamPatchBay.cpp
	src/ITAStreamProbe.cpp
	src/ITAStreamProperties.cpp
	src/ITAStreamYJunction.cpp
	src/ITADataSourceUtils.cpp
)
add_library (${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories (
	${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:INCLUDE_PATH/ITADataSources>
)

target_link_libraries (
	${PROJECT_NAME}
	PUBLIC ITABase::ITABase
	PRIVATE vista::vista_inter_proc_comm
)

# Filter windows-only components
if (WIN32)
	target_sources (
		${PROJECT_NAME} PRIVATE include/ITAStreamPump.h include/ITADataSourceUtils.h src/ITAStreamPump.cpp
								src/ITADataSourceUtils.cpp
	)
endif ()

if (TARGET asio::asio AND ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO)
	target_sources (${PROJECT_NAME} PRIVATE include/ITAAsioInterface.h src/ITAAsioInterface.cpp)
	# todo: Try to make this dependency private
	target_link_libraries (${PROJECT_NAME} PUBLIC asio::asio)
endif ()

if (TARGET portaudio AND ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO)
	target_sources (${PROJECT_NAME} PRIVATE include/ITAPortaudioInterface.h src/ITAPortaudioInterface.cpp)
	target_link_libraries (${PROJECT_NAME} PRIVATE portaudio)
endif ()

if (VJACK_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_JACK)
	message (FATAL_ERROR "Building with Jack still has to be implemented.")
	list (APPEND ITADataSourcesHeader "include/ITAJACKInterface.h")
	list (APPEND ITADataSourcesSources "src/ITAJACKInterface.cpp")
endif ()

if (ITA_DATA_SOURCES_WITH_NET_AUDIO)
	target_sources (
		${PROJECT_NAME}
		PRIVATE include/ITANetAudioStream.h
				include/ITANetAudioSampleServer.h
				include/ITANetAudioStreamingServer.h
				src/ITANetAudioClient.cpp
				src/ITANetAudioClient.h
				src/ITANetAudioMessage.cpp
				src/ITANetAudioMessage.h
				src/ITANetAudioProtocol.h
				src/ITANetAudioServer.cpp
				src/ITANetAudioServer.h
				src/ITANetAudioStream.cpp
				src/ITANetAudioStreamingClient.cpp
				src/ITANetAudioStreamingClient.h
				src/ITANetAudioStreamingServer.cpp
	)
endif ()

target_compile_definitions (
	${PROJECT_NAME} PUBLIC $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,ITA_DATA_SOURCES_EXPORT,ITA_DATA_SOURCES_STATIC>
)

target_compile_features (${PROJECT_NAME} PUBLIC cxx_std_11)

if (ITA_DATA_SOURCES_NET_AUDIO_SHOW_TRAFFIC)
	target_compile_definitions (${PROJECT_NAME} PRIVATE NET_AUDIO_SHOW_TRAFFIC)
endif ()

if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set_property (TARGET ${PROJECT_NAME} PROPERTY FOLDER "ITACoreLibs")
endif ()

packageProject (
	NAME ${PROJECT_NAME}
	VERSION ${PROJECT_VERSION}
	BINARY_DIR ${PROJECT_BINARY_DIR}
	INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
	INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ITADataSources
	DEPENDENCIES "ITABase;vista_inter_proc_comm"
	COMPATIBILITY ExactVersion
	NAMESPACE ${PROJECT_NAME}
	DISABLE_VERSION_SUFFIX YES
)

# apps
if (ITA_DATA_SOURCES_WITH_APPS)
	add_subdirectory (apps/ita_whad)
endif ()

# tests
if (ITA_DATA_SOURCES_WITH_TESTS)
	add_subdirectory (tests)
endif ()
