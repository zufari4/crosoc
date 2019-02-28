#pragma once
#include "crosoc.h"
#include <vector>
#define DEF_TIMEOUT 5
#define MAX_TCP_PACKET_SIZE 65535

class Tcp_connection
{
public:
    Tcp_connection();
    virtual ~Tcp_connection();
    virtual bool Open(const std::string& server, int port, int timeout_sec=DEF_TIMEOUT);
    void Close();
    int  Receive(char* data, int max_size, int timeout_sec=DEF_TIMEOUT, int timeout_usec = 0);
    int  Receive(std::string& data, int need_size = 0, int timeout_sec=DEF_TIMEOUT, int timeout_usec = 0);
    bool Send(const void* data, int size);
    bool Send(const std::string& data);
    const char* Get_last_error();
protected:
    bool m_open;
    SOCKET m_socket;
    std::string m_last_error;
    std::vector<char> m_tmp_buff;
    std::string m_server;
};