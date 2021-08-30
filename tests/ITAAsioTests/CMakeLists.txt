cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

add_executable (ListDevices ListDevices.cpp)
target_link_libraries (ListDevices PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ListDevices PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS ListDevices RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (Loopback Loopback.cpp)
target_link_libraries (Loopback PUBLIC ITADataSources::ITADataSources)

set_property (TARGET Loopback PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS Loopback RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (Play Play.cpp)
target_link_libraries (Play PUBLIC ITADataSources::ITADataSources)

set_property (TARGET Play PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS Play RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (Record Record.cpp)
target_link_libraries (Record PUBLIC ITADataSources::ITADataSources)

set_property (TARGET Record PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS Record RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ThreadInit ThreadInit.cpp)
target_link_libraries (ThreadInit PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ThreadInit PROPERTY FOLDER "Tests/ITADataSources/Asio")

install (TARGETS ThreadInit RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################
