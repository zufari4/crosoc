cmake_minimum_required(VERSION 2.8)

project(Crosoc)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
endif()

find_package(OpenSSL REQUIRED)

if(WIN32)
set(SOURCES
Crosoc.cpp
dllmain.cpp
http.cpp
shared.cpp
ssl_conn.cpp
tcp.cpp
udp.cpp
wsa_guard.cpp
Endpoint.cpp
)
else()
set(SOURCES
Crosoc.cpp
http.cpp
shared.cpp
ssl_conn.cpp
tcp.cpp
udp.cpp
Endpoint.cpp
)
endif()

add_library(Crosoc SHARED ${SOURCES})

if(WIN32)
  target_link_libraries(Crosoc user32 ws2_32 crypt32 advapi32 libssl libcrypto)
else() 
  target_link_libraries(Crosoc OpenSSL::SSL)
endif()
