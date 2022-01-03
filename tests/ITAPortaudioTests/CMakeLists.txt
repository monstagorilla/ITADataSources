cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

add_executable (ITAPortaudioInterfaceRecorder ITAPortaudioInterfaceRecorder.cpp)
target_link_libraries (ITAPortaudioInterfaceRecorder PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAPortaudioInterfaceRecorder PROPERTY FOLDER "Tests/ITADataSources/Portaudio")
set_property (
	TARGET ITAPortaudioInterfaceRecorder PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS ITAPortaudioInterfaceRecorder RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ITAPortaudioInterfaceTest ITAPortaudioInterfaceTest.cpp)
target_link_libraries (ITAPortaudioInterfaceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAPortaudioInterfaceTest PROPERTY FOLDER "Tests/ITADataSources/Portaudio")
set_property (TARGET ITAPortaudioInterfaceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAPortaudioInterfaceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ITAPortaudioMeasurementTest ITAPortaudioMeasurementTest.cpp)
target_link_libraries (ITAPortaudioMeasurementTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAPortaudioMeasurementTest PROPERTY FOLDER "Tests/ITADataSources/Portaudio")
set_property (
	TARGET ITAPortaudioMeasurementTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS ITAPortaudioMeasurementTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ReciprocalDAADLatencyMeasurement ReciprocalDAADLatencyMeasurement.cpp)
target_link_libraries (ReciprocalDAADLatencyMeasurement PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ReciprocalDAADLatencyMeasurement PROPERTY FOLDER "Tests/ITADataSources/Portaudio")
set_property (
	TARGET ReciprocalDAADLatencyMeasurement PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS ReciprocalDAADLatencyMeasurement RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
