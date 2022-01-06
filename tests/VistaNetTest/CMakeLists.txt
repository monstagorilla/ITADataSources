cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (ITADataSourcesTest)

# ######################################################################################################################

add_executable (ITAVistaNetTest VistaNetTest.cpp)
target_link_libraries (ITAVistaNetTest PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAVistaNetTest PROPERTY FOLDER "Tests/ITADataSources/VistaNet")
set_property (TARGET ITAVistaNetTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAVistaNetTest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ITAVistaNetClient VistaNetClient.cpp)
target_link_libraries (ITAVistaNetClient PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAVistaNetClient PROPERTY FOLDER "Tests/ITADataSources/VistaNet")
set_property (TARGET ITAVistaNetClient PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAVistaNetClient RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ######################################################################################################################

add_executable (ITAVistaNetServer VistaNetServer.cpp)
target_link_libraries (ITAVistaNetServer PUBLIC ITADataSources::ITADataSources)

set_property (TARGET ITAVistaNetServer PROPERTY FOLDER "Tests/ITADataSources/VistaNet")
set_property (TARGET ITAVistaNetServer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (TARGETS ITAVistaNetServer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
