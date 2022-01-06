cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

# Loopback
add_executable (ITAJackLoopback ITAJackLoopback.cpp)
target_link_libraries (ITAJackLoopback PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAJackLoopback PROPERTY FOLDER "Tests/ITADataSources/Jack")
set_property (TARGET ITAJackLoopback PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAJackLoopback RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

# Playback
add_executable (ITAJackPlayback ITAJackPlayback.cpp)
target_link_libraries (ITAJackPlayback PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAJackPlayback PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/Jack")
set_property (TARGET ITAJackPlayback PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAJackLoopback RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
