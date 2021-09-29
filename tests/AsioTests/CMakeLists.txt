cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (NativeAsioSingleInstanceTests)

# ######################################################################################################################

add_executable (NativeAsioSingleInstanceTest NativeAsioSingleInstanceTest.cpp)
target_link_libraries (NativeAsioSingleInstanceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET NativeAsioSingleInstanceTest PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS NativeAsioSingleInstanceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (NativeAsioThreadedMultiInstanceTest NativeAsioThreadedMultiInstanceTest.cpp)
target_link_libraries (NativeAsioThreadedMultiInstanceTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET NativeAsioThreadedMultiInstanceTest PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS NativeAsioThreadedMultiInstanceTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
