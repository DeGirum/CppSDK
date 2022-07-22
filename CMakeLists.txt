cmake_minimum_required( VERSION 3.12 )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

project( AICLIENTLIB VERSION 0.0.1 )

# Set up compiler
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
if(MSVC)
  add_compile_definitions( NOMINMAX )    # No min/max macros
endif()

# Set up paths
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

set( SRCPATH ${CMAKE_CURRENT_SOURCE_DIR}/client )
set( EXAMPLESPATH ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp )

set( INCPATH
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/inc/Utilities
	${CMAKE_CURRENT_SOURCE_DIR}/inc/DglibInterface
)

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
  	${SRCPATH}/dg_model_api.cpp
)

find_package( Threads REQUIRED )

add_library( aiclientlib STATIC ${DG_API_SRCs} ${INCPATH} )

target_include_directories( aiclientlib PUBLIC ${INCPATH} )

if( WIN32 )
	target_link_libraries( aiclientlib PUBLIC Ws2_32 )
endif()

add_subdirectory( ${EXAMPLESPATH} )
