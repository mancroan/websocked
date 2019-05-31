cmake_minimum_required(VERSION 2.8)
project(libwebsocket_server)

SET(CMAKE_BUILD_TYPE DEBUG)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	include_directories(
	${PROJECT_SOURCE_DIR}/include "/usr/local/include" "/usr/include"
	)
   link_directories(${PROJECT_SOURCE_DIR} "/usr/lib" "/usr/local/lib" "/usr/local/openssl/lib")
   link_libraries("websockets" "ssl" "crypto")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
 
IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")

	include_directories(
	${PROJECT_SOURCE_DIR}/include
	)

	LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})
	link_libraries("websockets_static" "ssleay32" "libeay32" "ws2_32" "wsock32")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")


add_executable(server server.cc)

