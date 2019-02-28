cmake_minimum_required(VERSION 2.8)

project(crosoc)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -g3")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

set(SOURCES
crosoc.cpp
crosoc.h
udp.cpp
udp.h
tcp.cpp
tcp.h
http.cpp
http.h
)

add_library(crosoc STATIC ${SOURCES})

if(WIN32)
  target_link_libraries(crosoc wsock32 ws2_32)
endif()