#include "Config.h"
#include "udp.h"
#include <string.h>

Udp_connection::Udp_connection():
    m_open(false),
    m_socket(INVALID_SOCKET)
{
    m_tmp_buff.resize(UDP_BUFFER_SIZE);
}

Udp_connection::~Udp_connection()
{
    Close();
}

bool Udp_connection::Open(int port /*= 0*/)
{
    m_last_error.clear();

    if (!Crosoc::Init()) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == INVALID_SOCKET)  {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }
    
    sockaddr_in addr;
    if (!Crosoc::Make_endpoint("", port, addr)) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }
    
    if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        m_last_error = Crosoc::Get_last_error();
        Close();
        return false;
    }

    m_open = true;
    return true;
}

void Udp_connection::Close()
{
    Crosoc::Close(m_socket);
    m_open = false;
}

int Udp_connection::Receive(void* data, int max_size, sockaddr_in* endpoint /*= nullptr*/)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return SOCKET_ERROR;
    }

    int recv_len;
    if (endpoint == nullptr) {
        recv_len = recv(m_socket, (char*)data, max_size, 0);
    }
    else {
        socklen_t slen = sizeof(sockaddr_in);
        recv_len = recvfrom(m_socket, (char*)data, max_size, 0, (sockaddr*)endpoint, &slen);
    }

    if (recv_len == SOCKET_ERROR) {
        m_last_error = Crosoc::Get_last_error();
    }

    return recv_len;
}

bool Udp_connection::Send(const void* data, int size, const sockaddr_in& endpoin)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return false;
    }

    int slen = sizeof(sockaddr_in);
    if (sendto(m_socket, (const char *)data, size, 0, (const sockaddr*)&endpoin, slen) == SOCKET_ERROR) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }

    return true;
}

bool Udp_connection::Send(const std::string& data, const sockaddr_in& endpoin)
{
    return Send(data.data(), (int)data.size(), endpoin);
}

const char* Udp_connection::Get_last_error()
{
    return m_last_error.c_str();
}
