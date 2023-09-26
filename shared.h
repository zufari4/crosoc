#pragma once

#include "net.h"
#include "ProtocolType.h"
#include <string>

#ifdef _WIN32
#define LIB_API2 __declspec(dllexport)
#else
#define LIB_API2 __attribute__((visibility("default")))
#endif

namespace Crosoc
{
    bool Init();
    LIB_API2 void Close(SOCKET& sock);
    LIB_API2 std::string Get_last_error();
    bool Make_endpoint(const std::string& host, int port, sockaddr_in& res, ProtocolType protoType = ProtocolType::TCP);
    std::string Get_host(const sockaddr_in& addr);
    int Get_port(const sockaddr_in& addr);
    void Set_nonblocking(SOCKET sock, bool nonblocking);
    bool Is_connection_error();
    bool Wait_socket_is_ready(SOCKET sock, long sec, long usec=0);
    int  Wait_data(SOCKET sock, long sec, long usec=0);
}