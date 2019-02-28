#pragma once
#include "crosoc.h"
#include <vector>

class Udp_connection
{
public:
    Udp_connection();
    ~Udp_connection();
    bool Open(int port = 0);
    void Close();
    int  Receive(void* data, int max_size, sockaddr_in* endpoint = nullptr);
    int  Receive(std::string& data, sockaddr_in* endpoint = nullptr);
    bool Send(const void* data, int size, const sockaddr_in& endpoin);
    bool Send(const std::string& data, const sockaddr_in& endpoin);
    const char* Get_last_error();
private:
    bool m_open;
    SOCKET m_socket;
    std::string m_last_error;
    std::vector<char> m_tmp_buff;
};