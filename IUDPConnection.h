#pragma once

#include <string>

struct sockaddr_in;

namespace Crosoc
{
    class IUDPConnection
    {
    public:
        IUDPConnection() {}
        virtual ~IUDPConnection() {}
        virtual bool Open(int port = 0) = 0;
        virtual void Close() = 0;
        virtual int  Receive(void* data, int max_size, sockaddr_in* endpoint = nullptr) = 0;
        virtual bool Send(const void* data, int size, const sockaddr_in& endpoin) = 0;
        virtual bool Send(const std::string& data, const sockaddr_in& endpoin) = 0;
        virtual const char* Get_last_error() = 0;
    };
}