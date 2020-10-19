#pragma once

#include "net.h"
#include <string>

namespace Crosoc
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