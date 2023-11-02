cmake_minimum_required( VERSION 3.12 )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

project( AICLIENTLIB VERSION 0.0.1 )

# Set up compiler
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
if(MSVC)
  # No min/max macros, disable deprecation warninigs
  add_compile_definitions( NOMINMAX _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601)
endif()

# Set up paths
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

set( SRCPATH ${CMAKE_CURRENT_SOURCE_DIR}/client )
set( EXAMPLESPATH ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp )
set( UTIL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc/Utilities )
set( INCPATH
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${UTIL_PATH}
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
	${SRCPATH}/dg_client_asio.cpp
	${SRCPATH}/dg_client_http.cpp
	${SRCPATH}/dg_model_api.cpp
	${UTIL_PATH}/easywsclient.cpp
	${UTIL_PATH}/dg_utility_singletons.cpp
)

find_package( Threads REQUIRED )

add_library( aiclientlib STATIC ${DG_API_SRCs} )

target_include_directories( aiclientlib PUBLIC ${INCPATH} )

if( WIN32 )
	target_link_libraries( aiclientlib PUBLIC Ws2_32 )
else()
	target_link_libraries( aiclientlib PUBLIC pthread dl )
endif()

add_subdirectory( ${EXAMPLESPATH} )
