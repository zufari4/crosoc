#pragma once
#include "ITCPConnection.h"
#include "shared.h"
#include <vector>

class Tcp_connection: public Crosoc::ITCPConnection
{
public:
    Tcp_connection();
    Tcp_connection(const Crosoc::Endpoint& client);
    virtual ~Tcp_connection();
    bool Connect(const std::string& server, int port, int timeout_sec) override;
    void Disconnect() override;
    int64_t Receive(void* out, int64_t outSize, int64_t need_size, int timeout_sec, int timeout_usec) override;
    int64_t ReceiveAll(void* out, int64_t outSize, int timeout_sec, int timeout_usec) override;
    bool Send(const void* data, int size) override;
    bool Send(const std::string& data) override;
    const char* GetLastError() const override;
    const std::string& GetServerHost() const override;
    void SetError(const std::string& error_msg) override;
    bool IsClosed() const override;
    virtual int64_t Receive(std::vector<uint8_t>& out, int64_t need_size, int timeout_sec = DEF_TIMEOUT, int timeout_usec = 0);
    bool Open(uint16_t port, int countListen) override;
    bool Accept(Crosoc::Endpoint& out) override;
protected:
    virtual int Recv(void* data, int64_t max_size, int timeout_sec, int timeout_usec = 0);
    bool m_open;
    SOCKET m_socket;
    std::string m_last_error;
    std::vector<char> m_tmp_buff;
    std::string m_server;
};