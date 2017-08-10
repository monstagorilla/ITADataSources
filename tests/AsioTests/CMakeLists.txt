cmake_minimum_required( VERSION 2.8 )

project( NativeAsioSingleInstanceTests )
list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
include( VistaCommon )
	
vista_use_package( asio REQUIRED FIND_DEPENDENCIES )
vista_use_package( VistaCoreLibs REQUIRED COMPONENTS VistaBase VistaInterProcComm FIND_DEPENDENCIES )

add_definitions( -DIEEE754_64FLOAT ) # ASIOSamplerate -> double

add_executable( NativeAsioSingleInstanceTest NativeAsioSingleInstanceTest.cpp )
target_link_libraries( NativeAsioSingleInstanceTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( NativeAsioSingleInstanceTest )
vista_install( NativeAsioSingleInstanceTest )
vista_create_default_info_file( NativeAsioSingleInstanceTest )

set_property( TARGET NativeAsioSingleInstanceTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/Asio" )

add_executable( NativeAsioThreadedMultiInstanceTest NativeAsioThreadedMultiInstanceTest.cpp )
target_link_libraries( NativeAsioThreadedMultiInstanceTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( NativeAsioThreadedMultiInstanceTest )
vista_install( NativeAsioThreadedMultiInstanceTest )
vista_create_default_info_file( NativeAsioThreadedMultiInstanceTest )

set_property( TARGET NativeAsioThreadedMultiInstanceTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/Asio" )

add_executable( ReciprocalDAADLatencyMeasurement ReciprocalDAADLatencyMeasurement.cpp )
target_link_libraries( ReciprocalDAADLatencyMeasurement ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( ReciprocalDAADLatencyMeasurement )
vista_install( ReciprocalDAADLatencyMeasurement )
vista_create_default_info_file( ReciprocalDAADLatencyMeasurement )

set_property( TARGET ReciprocalDAADLatencyMeasurement PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources/Asio" )
