#pragma once

#include "Config.h"
#include <unordered_map>
#include <string>

namespace Crosoc
{
    using HttpPostData = std::unordered_map<std::string, std::string>;

    class IHTTPConnection
    {
    public:
        IHTTPConnection(bool /*useSSL*/) {}
        virtual ~IHTTPConnection() {}
        virtual bool Connect(const std::string& server, int port = DEF_HTTP_PORT, int timeout_sec = DEF_TIMEOUT) = 0;
        virtual void Disconnect() = 0;
        virtual int64_t  Get(const std::string& path, char* out, int64_t outSize, int timeout_sec = DEF_TIMEOUT) = 0;
        virtual int64_t  Post(const std::string& path, const HttpPostData& data, char* out, int64_t outSize, int timeout_sec = DEF_TIMEOUT) = 0;
        virtual bool IsClosed() const = 0;
        virtual const char* GetLastError() const = 0;
        virtual int GetID() const = 0;
        virtual void addHeader(const std::string& name, const std::string& value) = 0;
    };

}