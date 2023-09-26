#include "ssl_conn.h"
#include "openssl/err.h"

Ssl_connection::Ssl_connection() :
    Tcp_connection(),
    m_ctx(nullptr),
    m_ssl(nullptr),
    m_ssl_socket(INVALID_SOCKET)
{

}

bool Ssl_connection::Connect(const std::string& server, int port, int timeout_sec /*= DEF_TIMEOUT*/)
{
    if (!Tcp_connection::Connect(server, port, timeout_sec)) {
        return false;
    }
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    const SSL_METHOD* meth = TLS_client_method();
#else
    const SSL_METHOD* meth = TLSv1_2_client_method();
#endif
    
    m_ctx = SSL_CTX_new(meth);
    m_ssl = SSL_new(m_ctx);
    if (!m_ssl) {
        Log_ssl_error();
        return false;
    }
    m_ssl_socket = SSL_get_fd(m_ssl);
    SSL_set_fd(m_ssl, (int)m_socket);
    int err = SSL_connect(m_ssl);
    if (err <= 0) {
        Log_ssl_error();
        return false;
    }
    return true;
}

void Ssl_connection::Disconnect()
{
    Crosoc::Close(m_ssl_socket);
    if (m_ssl != nullptr) SSL_shutdown(m_ssl);
    if (m_ssl != nullptr) SSL_free(m_ssl);
    if (m_ctx != nullptr) SSL_CTX_free(m_ctx);
    Tcp_connection::Disconnect();
}

int Ssl_connection::Recv(void* data, int64_t max_size, int /* timeout_sec = DEF_TIMEOUT*/, int /*timeout_usec = 0*/)
{
    if (!m_open) {
        m_last_error = "Connection not open";
        return SOCKET_ERROR;
    }

//     if (crosoc::Wait_data(m_socket, timeout_sec, timeout_usec) <= 0) {
//         m_last_error = "Receive time out";
//         Close();
//         return SOCKET_ERROR;
//     }

    int recv_len = SSL_read(m_ssl, data, (int)(max_size > MAX_TCP_PACKET_SIZE ? MAX_TCP_PACKET_SIZE : max_size));
    if (recv_len <= 0) {
        Log_ssl_error();
        Disconnect();
    }
    if (recv_len == 0) {
        m_last_error = "Connection droped";
        Disconnect();
    }
    return recv_len;
}

bool Ssl_connection::Send(const void* data, int size)
{
    if (SSL_write(m_ssl, data, size) <= 0) {
        Log_ssl_error();
        return false;
    }
    return true;
}

void Ssl_connection::Log_ssl_error()
{
    int err = ERR_get_error();
    std::string strErr;
    char buff[1024];

    while (err) {
        buff[0] = '\0';
        ERR_error_string_n(err, buff, sizeof(buff));
        strErr += buff;
        err = ERR_get_error();
    }
    if (!strErr.empty()) {
        m_last_error = strErr;
    }
}
