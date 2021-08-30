set (CPM_DOWNLOAD_VERSION 0.32.2)
set (CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
	message (STATUS "Downloading CPM.cmake")
	file (DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
		  ${CPM_DOWNLOAD_LOCATION}
	)
else (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
	message ("CPM already exists, do not need to download")
endif (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))

include (${CPM_DOWNLOAD_LOCATION})

CPMAddPackage (
	NAME asio
	URL https://www.steinberg.net/asiosdk
	DOWNLOAD_ONLY
	#OPTIONS ""
)

if (asio_ADDED)
	add_library (asio STATIC
		${asio_SOURCE_DIR}/common/asio.cpp
		${asio_SOURCE_DIR}/common/asio.h
		${asio_SOURCE_DIR}/host/asiodrivers.cpp
		${asio_SOURCE_DIR}/host/asiodrivers.h
		${asio_SOURCE_DIR}/host/ginclude.h
		${asio_SOURCE_DIR}/common/asiodrvr.h
		${asio_SOURCE_DIR}/common/combase.cpp
		${asio_SOURCE_DIR}/common/combase.h
		${asio_SOURCE_DIR}/common/dllentry.cpp
		${asio_SOURCE_DIR}/common/iasiodrv.h
		${asio_SOURCE_DIR}/common/register.cpp
		${asio_SOURCE_DIR}/common/wxdebug.h
	)
	add_library (asio::asio ALIAS asio)

	target_include_directories (
		asio PUBLIC $<BUILD_INTERFACE:${asio_SOURCE_DIR}> $<BUILD_INTERFACE:${asio_SOURCE_DIR}/common> $<BUILD_INTERFACE:${asio_SOURCE_DIR}/host> $<INSTALL_INTERFACE:INCLUDE_PATH>
	)

	if (WIN32)
		target_include_directories (
			asio PUBLIC $<BUILD_INTERFACE:${asio_SOURCE_DIR}/host/pc> $<INSTALL_INTERFACE:INCLUDE_PATH>
		)
		target_sources(asio PRIVATE ${asio_SOURCE_DIR}/host/pc/asiolist.h ${asio_SOURCE_DIR}/host/pc/asiolist.cpp)
		target_compile_definitions(asio PUBLIC IEEE754_64FLOAT)
	endif()

	set_property (TARGET asio PROPERTY FOLDER "external_libs")

	packageProject (
		NAME asio
		VERSION 2.3.3
		BINARY_DIR ${PROJECT_BINARY_DIR}
		INCLUDE_DIR ${asio_SOURCE_DIR}/common
		INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		NAMESPACE asio
		DISABLE_VERSION_SUFFIX YES
	)
endif()

CPMAddPackage (
	NAME portaudio
	GITHUB_REPOSITORY PortAudio/portaudio
	GIT_TAG v19.7.0
	OPTIONS "PA_BUILD_STATIC OFF" "PA_DISABLE_INSTALL ON"
)

if (portaudio_ADDED)
	set_target_properties (portaudio PROPERTIES FOLDER "external_libs")

	packageProject (
		NAME portaudio
		VERSION 19.7.0
		BINARY_DIR ${PROJECT_BINARY_DIR}
		INCLUDE_DIR ${portaudio_SOURCE_DIR}/include
		INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		COMPATIBILITY ExactVersion
		NAMESPACE portaudio
		DEPENDENCIES ""
		DISABLE_VERSION_SUFFIX YES
	)
endif()
