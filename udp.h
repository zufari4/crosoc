#pragma once

#include "IUDPConnection.h"
#include "shared.h"
#include <vector>

class Udp_connection: public Crosoc::IUDPConnection
{
public:
    Udp_connection();
    virtual ~Udp_connection();
    bool Open(int port = 0) override;
    void Close() override;
    int  Receive(void* data, int max_size, sockaddr_in* endpoint) override;
    bool Send(const void* data, int size, const sockaddr_in& endpoin) override;
    bool Send(const std::string& data, const sockaddr_in& endpoin) override;
    const char* Get_last_error() override;
private:
    bool m_open;
    SOCKET m_socket;
    std::string m_last_error;
    std::vector<char> m_tmp_buff;
};