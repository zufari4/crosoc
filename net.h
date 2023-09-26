#pragma once

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <winsock2.h>
#include <ws2tcpip.h>

using socklen_t = int;

#else

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
using SOCKET = int;

#endif