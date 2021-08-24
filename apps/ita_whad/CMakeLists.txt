cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project (ita_whad)

add_executable (ita_whad main.cpp)
target_link_libraries (ita_whad ITADataSources::ITADataSources)

if (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO)
	target_compile_definitions (${PROJECT_NAME} PRIVATE ITA_WHAD_WITH_ASIO)
endif ()

if (ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO)
	target_compile_definitions (${PROJECT_NAME} PRIVATE ITA_WHAD_WITH_PORTAUDIO)
endif ()

set_property (TARGET ita_whad PROPERTY FOLDER "Apps/ITADataSources")

install (TARGETS ita_whad RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
