cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

# Loopback
add_executable (ITAJackLoopback ITAJackLoopback.cpp)
target_link_libraries (ITAJackLoopback PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAJackLoopback PROPERTY FOLDER "Tests/ITADataSources/Jack")

install (TARGETS ITAJackLoopback RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

# Playback
add_executable (ITAJackPlayback ITAJackPlayback.cpp)
target_link_libraries (ITAJackPlayback PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAJackPlayback PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/Jack")

install (TARGETS ITAJackLoopback RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
