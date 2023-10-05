#pragma once
#include "ITCPConnection.h"
#include "IHTTPConnection.h"
#include <memory>
#include <map>

class Http_connection: public Crosoc::IHTTPConnection
{
public:
    Http_connection(bool useSSL = false);
    virtual ~Http_connection();
    bool Connect(const std::string& server, int port, int timeout_sec) override;
    void Disconnect() override;
    int64_t Get(const std::string& path, char* out, int64_t outSize, int timeout_sec) override;
    int64_t Post(const std::string& path, const Crosoc::HttpPostData& data, char* out, int64_t outSize, int timeout_sec) override;
	bool IsClosed() const override;
    const char* GetLastError() const override;
    int GetID() const override;
    void addHeader(const std::string& name, const std::string& value) override;
private:
    int64_t  Receive_header(std::string& header, int timeout_sec = DEF_TIMEOUT);
    int64_t  Recive_chunk_size(int64_t& chunk_size, int timeout_sec = DEF_TIMEOUT);
    int64_t  Get_body_length(const std::string& header);
	bool Is_connection_closed(const std::string& header);
    bool Is_chunked(const std::string& header) const;
    std::string Make_get_message(const std::string& path) const;
    std::string Make_post_message(const std::string& path, const Crosoc::HttpPostData& data) const;
    int64_t Receive_data(const std::string& message, char* out, int64_t outSize, int timeout_sec);
    static std::string URLEncode(const std::string& s);

    bool useSSL_;
    std::unique_ptr<Crosoc::ITCPConnection> m_conn;
    tm tmptm_;
    int id_;
    std::map<std::string, std::string> headers_;
};