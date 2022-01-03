cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

add_executable (BufferDataSourceTest BufferDataSourceTest.cpp)
target_link_libraries (BufferDataSourceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET BufferDataSourceTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET BufferDataSourceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS BufferDataSourceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (FileDataSourceTest FileDataSourceTest.cpp)
target_link_libraries (FileDataSourceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET FileDataSourceTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET FileDataSourceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS FileDataSourceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (DatasourceUtilsTest DatasourceUtilsTest.cpp)
target_link_libraries (DatasourceUtilsTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET DatasourceUtilsTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET DatasourceUtilsTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS DatasourceUtilsTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (StreamDetectorTest StreamDetectorTest.cpp)
target_link_libraries (StreamDetectorTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET StreamDetectorTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET StreamDetectorTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS StreamDetectorTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (StreamFunctionGeneratorTest StreamFunctionGeneratorTest.cpp)
target_link_libraries (StreamFunctionGeneratorTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET StreamFunctionGeneratorTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (
	TARGET StreamFunctionGeneratorTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS StreamFunctionGeneratorTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (StreamPatchbayTest StreamPatchbayTest.cpp)
target_link_libraries (StreamPatchbayTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET StreamPatchbayTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET StreamPatchbayTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS StreamPatchbayTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (StreamProbeTest StreamProbeTest.cpp)
target_link_libraries (StreamProbeTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET StreamProbeTest PROPERTY FOLDER "Tests/ITADataSources")
set_property (TARGET StreamProbeTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS StreamProbeTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

# tests
if (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO)
	add_subdirectory (ITAAsioTests)
	add_subdirectory (AsioTests)
endif (ITA_DATA_SOURCES_WITH_INTEGRATED_ASIO)

if (ITA_DATA_SOURCES_WITH_INTEGRATED_PORTAUDIO)
	add_subdirectory (ITAPortaudioTests)
endif ()

if (VJACK_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_JACK)
	message (WARNING "Jack support is not tested")
	add_subdirectory (ITAJACKTests)
endif (VJACK_FOUND AND ITA_DATA_SOURCES_WITH_INTEGRATED_JACK)

add_subdirectory (NetAudio)
add_subdirectory (VistaNetTest)
