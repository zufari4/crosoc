#pragma once
#include "tcp.h"
#include <openssl/ssl.h>

class Ssl_connection : public Tcp_connection
{
public:
    Ssl_connection();
    bool Connect(const std::string& server, int port, int timeout_sec) override;
    void Disconnect() override;
    bool Send(const void* data, int size) override;
protected:
    int Recv(void* data, int64_t max_size, int timeout_sec, int timeout_usec = 0) override;
private:
    void Log_ssl_error();
    SSL_CTX* m_ctx;
    SSL* m_ssl;
    SOCKET m_ssl_socket;
};