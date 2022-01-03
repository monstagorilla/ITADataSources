cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (NativeAsioSingleInstanceTests)

# ######################################################################################################################

add_executable (NativeAsioSingleInstanceTest NativeAsioSingleInstanceTest.cpp)
target_link_libraries (NativeAsioSingleInstanceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET NativeAsioSingleInstanceTest PROPERTY FOLDER "Tests/ITADataSources/Asio")
set_property (
	TARGET NativeAsioSingleInstanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS NativeAsioSingleInstanceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (NativeAsioThreadedMultiInstanceTest NativeAsioThreadedMultiInstanceTest.cpp)
target_link_libraries (NativeAsioThreadedMultiInstanceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET NativeAsioThreadedMultiInstanceTest PROPERTY FOLDER "Tests/ITADataSources/Asio")
set_property (
	TARGET NativeAsioThreadedMultiInstanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

install (TARGETS NativeAsioThreadedMultiInstanceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
