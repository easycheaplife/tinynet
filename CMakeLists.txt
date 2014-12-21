PROJECT(tinynet)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

message(STATUS "current system is ${CMAKE_SYSTEM_NAME}")

IF(UNIX)
    IF("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
        message(STATUS "Darwin STREQUAL ${CMAKE_SYSTEM_NAME}")
        SET(TINYNET_SRC_LIST reactor.cc reactor_impl_select.cc event_handle_srv.cc  server_impl.cc
        event_handle_cli.cc client_impl.cc)
    ELSEIF("Linux" STREQUAL ${CMAKE_SYSTEM_NAME})
        message(STATUS "Linux STREQUAL ${CMAKE_SYSTEM_NAME}")
        SET(TINYNET_SRC_LIST reactor.cc reactor_impl_select.cc reactor_impl_poll.cc reactor_impl_epoll.cc
        event_handle_srv.cc  server_impl.cc event_handle_cli.cc client_impl.cc)
    ENDIF()
ELSE(WIN32)
	SET(TINYNET_SRC_LIST reactor.h reactor.cc reactor_impl_select.h reactor_impl_select.cc reactor_impl_iocp.h reactor_impl_iocp.cc
	event_handle_srv.h event_handle_srv.cc server_impl.h server_impl.cc event_handle_cli.h event_handle_cli.cc client_impl.h client_impl.cc)
ENDIF(UNIX)

INCLUDE_DIRECTORIES( . ../easy/src/base . )

IF(UNIX)
    IF("Darwin" STREQUAL ${CMAKE_SYSTEM_NAME})
        ADD_DEFINITIONS(-D__HAVE_SELECT -D__MACX)
    ELSEIF("Linux" STREQUAL ${CMAKE_SYSTEM_NAME})
        ADD_DEFINITIONS(-D__HAVE_SELECT -D__LINUX)
    ENDIF()
	SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
	SET(CMAKE_CXX_FLAGS "-Wl,--no-as-needed -std=c++11 -pthread") 
ELSE(WIN32)
	ADD_DEFINITIONS(-D__WIN32 -DWIN32)
ENDIF(UNIX)

ADD_LIBRARY(tinynet STATIC ${TINYNET_SRC_LIST})
SET_TARGET_PROPERTIES(tinynet PROPERTIES OUTPUT_NAME "tinynet")
SET(LIBRARY_OUTPUT_PATH lib)
ADD_SUBDIRECTORY( srv_test )
ADD_SUBDIRECTORY( cli_test )

