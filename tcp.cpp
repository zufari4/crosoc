#include "tcp.h"
#include <string.h>

Tcp_connection::Tcp_connection():
    m_open(false),
    m_socket(INVALID_SOCKET)
{
    m_tmp_buff.resize(MAX_TCP_PACKET_SIZE);
}

Tcp_connection::~Tcp_connection()
{
    Close();
}

bool Tcp_connection::Open(const std::string& server, int port, int timeout_sec /*= 5*/)
{
    m_last_error.clear();

    if (!crosoc::Init()) {
        m_last_error = crosoc::Get_last_error();
        return false;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET)  {
        m_last_error = crosoc::Get_last_error();
        return false;
    }
    
    sockaddr_in addr;
    if (!crosoc::Make_endpoint(server, port, addr)) {
        m_last_error = crosoc::Get_last_error();
        return false;
    }

    crosoc::Set_nonblocking(m_socket, true);

    if (connect(m_socket, (struct sockaddr *)&addr , sizeof(addr)) < 0) {
        if (crosoc::Is_connection_error()) {
            m_last_error = crosoc::Get_last_error();
            Close();
            return false;
        }
        if (!crosoc::Wait_socket_is_ready(m_socket, timeout_sec)) {
            m_last_error = "Connection time out";
            Close();
            return false;
        }
    }

    crosoc::Set_nonblocking(m_socket, false);

    m_server = server;
    m_open   = true;
    return true;
}

void Tcp_connection::Close()
{
    crosoc::Close(m_socket);
    m_open = false;
}

int Tcp_connection::Receive(char* data, int max_size, int timeout_sec /*=5*/, int timeout_usec /*= 0*/)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return SOCKET_ERROR;
    }

    if (crosoc::Wait_data(m_socket, timeout_sec, timeout_usec) <= 0) {
        m_last_error = "Receive time out";
        Close();
        return SOCKET_ERROR;
    }

    int recv_len  = recv(m_socket, data, max_size, 0);
    if (recv_len < 0) {
        m_last_error = crosoc::Get_last_error();
        Close();
    }
	if (recv_len == 0) {
		m_last_error = "Connection droped";
		Close();
	}
    return recv_len;
}

int Tcp_connection::Receive(std::string& data, int need_size /*= 0*/, int timeout_sec /*=5*/, int timeout_usec /*= 0*/)
{
    int rc_len = 0;
    int buf_size = (int)m_tmp_buff.size() - 1;
    int max_size = need_size == 0 ? buf_size : need_size;
    int len;
    data.clear();

    do 
    {
        len = Receive(&m_tmp_buff[0], buf_size, timeout_sec, timeout_usec);
        if (len > 0) {
            m_tmp_buff[len] = '\0';
            data += &m_tmp_buff[0];
            rc_len += len;
        }
    }
    while (len > 0 && rc_len < max_size);

    return rc_len;
}

bool Tcp_connection::Send(const void* data, int size)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return false;
    }

    if (send(m_socket, (const char*)data , size, 0) == SOCKET_ERROR) {
        m_last_error = crosoc::Get_last_error();
        return false;
    }

    return true;
}

bool Tcp_connection::Send(const std::string& data)
{
    return Send(data.data(), (int)data.length());
}

const char* Tcp_connection::Get_last_error()
{
    return m_last_error.c_str();
}
