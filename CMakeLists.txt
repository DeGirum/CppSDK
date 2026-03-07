cmake_minimum_required( VERSION 3.12 )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

project( AICLIENTLIB VERSION 0.0.1 )

option( BUILD_EXAMPLES "Build examples" ON )

# Set up compiler
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Set up paths
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

set( SRCPATH ${CMAKE_CURRENT_SOURCE_DIR}/client )
set( EXAMPLESPATH ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp )
set( UTIL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc/Utilities )

# Get dependency
if( WIN32 )
	execute_process ( COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/loadDependency.bat
					  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ )
else()
	execute_process ( COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/loadDependency.sh
					  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ )
endif()

# Set up API library
set( DG_API_SRCs
	${SRCPATH}/dg_client.cpp
	${SRCPATH}/dg_client_asio.cpp
	${SRCPATH}/dg_client_http.cpp
	${SRCPATH}/dg_model_api.cpp
	${UTIL_PATH}/easywsclient.cpp
	${UTIL_PATH}/dg_utility_singletons.cpp
)

find_package( Threads REQUIRED )

add_library( aiclientlib STATIC ${DG_API_SRCs} )

if(MSVC)
  # No min/max macros, disable deprecation warninigs
  target_compile_definitions( aiclientlib PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601)
  target_compile_options( aiclientlib PUBLIC /wd4267) # Allow implicit narrowing conversion from size_t
endif()

target_include_directories( aiclientlib PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
	$<BUILD_INTERFACE:${UTIL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/DglibInterface>
	$<INSTALL_INTERFACE:include>
)

if( WIN32 )
	target_link_libraries( aiclientlib PUBLIC Ws2_32 )
else()
	target_link_libraries( aiclientlib PUBLIC pthread dl )
endif()

if( BUILD_EXAMPLES )
	add_subdirectory( ${EXAMPLESPATH} )
endif()

install( TARGETS aiclientlib
	EXPORT aiclientlibTargets
	ARCHIVE DESTINATION lib/$<CONFIG>
	LIBRARY DESTINATION lib/$<CONFIG>
)
install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/ DESTINATION include )

# Generate and install CMake package config files
include( CMakePackageConfigHelpers )

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/aiclientlibConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

file( WRITE "${CMAKE_CURRENT_BINARY_DIR}/aiclientlibConfig.cmake"
	"include(\"\${CMAKE_CURRENT_LIST_DIR}/aiclientlibTargets.cmake\")\n"
)

install( EXPORT aiclientlibTargets
	FILE aiclientlibTargets.cmake
	NAMESPACE aiclientlib::
	DESTINATION lib/cmake/aiclientlib
)

install( FILES
	"${CMAKE_CURRENT_BINARY_DIR}/aiclientlibConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/aiclientlibConfigVersion.cmake"
	DESTINATION lib/cmake/aiclientlib
)
