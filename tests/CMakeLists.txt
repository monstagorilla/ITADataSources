cmake_minimum_required( VERSION 2.8 )

if( NOT ITADATASOURCES_COMMON_BUILD )

	project( ITADataSourcesTest )
	list( APPEND CMAKE_MODULE_PATH "$ENV{VISTA_CMAKE_COMMON}" )
	include( VistaCommon )
	
endif()

vista_use_package( ITADataSources REQUIRED FIND_DEPENDENCIES )

if( ITA_CORE_LIBS_BUILD_STATIC AND DEFINED ITA_CORE_LIBS_BUILD_STATIC )
	add_definitions( -DITA_BASE_STATIC -DITA_DATA_SOURCES_STATIC )
endif( ITA_CORE_LIBS_BUILD_STATIC AND DEFINED ITA_CORE_LIBS_BUILD_STATIC  )

# todo fix with if( vista_staic )!
add_definitions( -DVISTABASE_STATIC -DVISTAMATH_STATIC -DVISTAASPECTS_STATIC -DVISTATOOLS_STATIC -DVISTAINTERPROCCOMM_STATIC )

add_executable( BufferDataSourceTest BufferDataSourceTest.cpp )
target_link_libraries( BufferDataSourceTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( BufferDataSourceTest )
vista_install( BufferDataSourceTest )
vista_create_default_info_file( BufferDataSourceTest )

set_property( TARGET BufferDataSourceTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( FileDataSourceTest FileDataSourceTest.cpp )
target_link_libraries( FileDataSourceTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( FileDataSourceTest )
vista_install( FileDataSourceTest )
vista_create_default_info_file( FileDataSourceTest )

set_property( TARGET FileDataSourceTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( DatasourceUtilsTest DatasourceUtilsTest.cpp )
target_link_libraries( DatasourceUtilsTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( DatasourceUtilsTest )
vista_install( DatasourceUtilsTest )
vista_create_default_info_file( DatasourceUtilsTest )

set_property( TARGET DatasourceUtilsTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


# add_executable( FilterTest FilterTest.cpp )
# target_link_libraries( FilterTest ${VISTA_USE_PACKAGE_LIBRARIES} )

# vista_configure_app( FilterTest )
# vista_install( FilterTest )
# vista_create_default_info_file( FilterTest )

# set_property( TARGET FilterTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


# add_executable( Patchbay1Test Patchbay1Test.cpp )
# target_link_libraries( Patchbay1Test ${VISTA_USE_PACKAGE_LIBRARIES} )

# vista_configure_app( Patchbay1Test )
# vista_install( Patchbay1Test )
# vista_create_default_info_file( Patchbay1Test )

# set_property( TARGET Patchbay1Test PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( PeakDetectorTest PeakDetectorTest.cpp )
target_link_libraries( PeakDetectorTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( PeakDetectorTest )
vista_install( PeakDetectorTest )
vista_create_default_info_file( PeakDetectorTest )

set_property( TARGET PeakDetectorTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( StreamFunctionGeneratorTest StreamFunctionGeneratorTest.cpp )
target_link_libraries( StreamFunctionGeneratorTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( StreamFunctionGeneratorTest )
vista_install( StreamFunctionGeneratorTest )
vista_create_default_info_file( StreamFunctionGeneratorTest )

set_property( TARGET StreamFunctionGeneratorTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( StreamPatchbayTest StreamPatchbayTest.cpp )
target_link_libraries( StreamPatchbayTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( StreamPatchbayTest )
vista_install( StreamPatchbayTest )
vista_create_default_info_file( StreamPatchbayTest )

set_property( TARGET StreamPatchbayTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


add_executable( StreamProbeTest StreamProbeTest.cpp )
target_link_libraries( StreamProbeTest ${VISTA_USE_PACKAGE_LIBRARIES} )

vista_configure_app( StreamProbeTest )
vista_install( StreamProbeTest )
vista_create_default_info_file( StreamProbeTest )

set_property( TARGET StreamProbeTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


# add_executable( YJunctionTest YJunctionTest.cpp )
# target_link_libraries( YJunctionTest ${VISTA_USE_PACKAGE_LIBRARIES} )

# vista_configure_app( YJunctionTest )
# vista_install( YJunctionTest )
# vista_create_default_info_file( YJunctionTest )

# set_property( TARGET YJunctionTest PROPERTY FOLDER "ITACoreLibs/Tests/ITADataSources" )


# tests
if( ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )
	add_subdirectory( ITAAsioTests )
endif( ITA_DATA_SOURCE_WITH_INTEGRATED_ASIO )

if( ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )
	add_subdirectory( "ITAPortaudioTests" )
endif( ITA_DATA_SOURCE_WITH_INTEGRATED_PORTAUDIO )

if( VJACK_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_JACK )
	add_subdirectory( "ITAJACKTests" )
endif( VJACK_FOUND AND ITA_DATA_SOURCE_WITH_INTEGRATED_JACK )

add_subdirectory( "NetAudio" )
add_subdirectory( "VistaNetTest" )
