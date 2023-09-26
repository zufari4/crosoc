#include "tcp.h"
#include "Endpoint.h"
#include <string.h>
#include <algorithm>

Tcp_connection::Tcp_connection():
    m_open(false),
    m_socket(INVALID_SOCKET)
{
    m_tmp_buff.resize(MAX_TCP_PACKET_SIZE);
}

Tcp_connection::Tcp_connection(const Crosoc::Endpoint& client)
    : ITCPConnection(client)
    , m_open(client.socket != INVALID_SOCKET)
    , m_socket(client.socket)
    
{
    m_tmp_buff.resize(MAX_TCP_PACKET_SIZE);
}

Tcp_connection::~Tcp_connection()
{
    Disconnect();
}

bool Tcp_connection::Open(uint16_t port, int countListen)
{
    m_last_error.clear();
    m_open = false;

    if (!Crosoc::Init()) {
        m_last_error = "Init crosoc: " + Crosoc::Get_last_error();
        return false;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        m_last_error = "Create socket: " + Crosoc::Get_last_error();
        return false;
    }

    sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    address.sin_family = AF_INET;

    if (bind(m_socket, (struct sockaddr*)&address, sizeof(address)) == INVALID_SOCKET) {
        m_last_error = "Bind tcp: " + Crosoc::Get_last_error();
        return false;
    }

    if (listen(m_socket, countListen) == INVALID_SOCKET) {
        m_last_error = m_last_error = "listen: " + Crosoc::Get_last_error();
        return false;
    }

    m_open = true;
    return true;
}

bool Tcp_connection::Accept(Crosoc::Endpoint& out)
{
    socklen_t addrlen = sizeof(out.addr);
    out.socket = accept(m_socket, (struct sockaddr*)&out.addr, &addrlen);
    if (out.socket == INVALID_SOCKET) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }
    return true;
}

bool Tcp_connection::Connect(const std::string& server, int port, int timeout_sec /*= 5*/)
{
    m_last_error.clear();

    if (!Crosoc::Init()) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET)  {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }
    
    sockaddr_in addr;
    if (!Crosoc::Make_endpoint(server, port, addr)) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }

    Crosoc::Set_nonblocking(m_socket, true);

    if (connect(m_socket, (struct sockaddr *)&addr , sizeof(addr)) < 0) {
        if (Crosoc::Is_connection_error()) {
            m_last_error = Crosoc::Get_last_error();
            Disconnect();
            return false;
        }
        if (!Crosoc::Wait_socket_is_ready(m_socket, timeout_sec)) {
            m_last_error = "Connection time out";
            Disconnect();
            return false;
        }
    }

    Crosoc::Set_nonblocking(m_socket, false);

    m_server = server;
    m_open   = true;
    return true;
}

void Tcp_connection::Disconnect()
{
    Crosoc::Close(m_socket);
    m_open = false;
}

int Tcp_connection::Recv(void* data, int64_t max_size, int timeout_sec /*=5*/, int timeout_usec /*= 0*/)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return SOCKET_ERROR;
    }

    if (Crosoc::Wait_data(m_socket, timeout_sec, timeout_usec) <= 0) {
        m_last_error = "Receive time out";
        Disconnect();
        return SOCKET_ERROR;
    }

    int recv_len  = recv(m_socket, static_cast<char*>(data), (int)(max_size > MAX_TCP_PACKET_SIZE ? MAX_TCP_PACKET_SIZE : max_size), 0);
    if (recv_len < 0) {
        m_last_error = Crosoc::Get_last_error();
        Disconnect();
    }
	if (recv_len == 0) {
		m_last_error = "Connection droped";
		Disconnect();
	}
    return recv_len;
}

int64_t Tcp_connection::Receive(void* out, int64_t outSize, int64_t need_size, int timeout_sec /*=5*/, int timeout_usec /*= 0*/)
{
    if (out == nullptr) {
        m_last_error = "Destination buffer is empty";
        return 0;
    }
    if (need_size == 0) {
        return ReceiveAll(out, outSize, timeout_sec, timeout_usec);
    }
    int len;
    int64_t offset = 0;
    int64_t iter_size;

    do
    {
        if (offset >= outSize && need_size > 0) {
            len = Recv(&m_tmp_buff[0], need_size, timeout_sec, timeout_usec);
            if (len > 0) {
                need_size -= len;
            }
        }
        else {
            iter_size = std::min(need_size, outSize - offset);
            len = Recv((char*)out + offset, iter_size, timeout_sec, timeout_usec);
            if (len > 0) {
                need_size -= len;
                offset += len;
            }
        }

    } while (len > 0 && need_size > 0);

    return offset;
}

int64_t Tcp_connection::ReceiveAll(void* out, int64_t outSize, int timeout_sec, int timeout_usec)
{
    int len;
    int64_t offset = 0;
    int64_t sz;

    do
    {
        if (offset >= outSize) {
            len = Recv(&m_tmp_buff[0], MAX_TCP_PACKET_SIZE, timeout_sec, timeout_usec);
        }
        else {
            sz = std::min((int64_t)MAX_TCP_PACKET_SIZE, outSize - offset);
            len = Recv((char*)out + offset, sz, timeout_sec, timeout_usec);
            if (len > 0) {
                offset += len;
            }
        }
    } while (len > 0);

    return offset;
}

int64_t Tcp_connection::Receive(std::vector<uint8_t>& out, int64_t need_size, int timeout_sec /*= DEF_TIMEOUT*/, int timeout_usec /*= 0*/)
{
    int len;
    int64_t all_len = 0;

    do
    {
        len = Recv(&m_tmp_buff[0], need_size, timeout_sec, timeout_usec);
        if (len > 0) {
            out.insert(out.end(), m_tmp_buff.begin(), m_tmp_buff.begin() + len);
            need_size -= len;
            all_len += len;
        }
    } while (len > 0 && need_size > 0);

    return all_len;
}

bool Tcp_connection::Send(const void* data, int size)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return false;
    }

    if (send(m_socket, (const char*)data , size, 0) == SOCKET_ERROR) {
        m_last_error = Crosoc::Get_last_error();
        return false;
    }

    return true;
}

bool Tcp_connection::Send(const std::string& data)
{
    return Send(data.data(), (int)data.length());
}

const char* Tcp_connection::GetLastError() const
{
    return m_last_error.c_str();
}

const std::string& Tcp_connection::GetServerHost() const
{
    return m_server;
}

void Tcp_connection::SetError(const std::string& error_msg)
{
    m_last_error = error_msg;
}

bool Tcp_connection::IsClosed() const
{
    return !m_open;
}
