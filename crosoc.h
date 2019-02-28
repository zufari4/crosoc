#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#endif

#ifdef __linux__
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
typedef int SOCKET;
#endif

#include <string>

namespace crosoc
{
    bool Init();
    void Close(SOCKET& sock);
    std::string Get_last_error();
    bool Make_endpoint(const std::string& host, int port, sockaddr_in& res);
    std::string Get_host(const sockaddr_in& addr);
    int Get_port(const sockaddr_in& addr);
    void Set_nonblocking(SOCKET sock, bool nonblocking);
    bool Is_connection_error();
    bool Wait_socket_is_ready(SOCKET sock, long sec, long usec=0);
    int  Wait_data(SOCKET sock, long sec, long usec=0);
}