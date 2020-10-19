#pragma once

#include "Config.h"
#include <string>
#include <inttypes.h>

namespace Crosoc
{
	class ITCPConnection
	{
	public:
		ITCPConnection() {}
		virtual ~ITCPConnection() {}
        virtual bool Connect(const std::string& server, int port, int timeout_sec = DEF_TIMEOUT) = 0;
        virtual void Disconnect() = 0;
        virtual int64_t Receive(void* out, int64_t outSize, int64_t need_size, int timeout_sec = DEF_TIMEOUT, int timeout_usec = 0) = 0;
        virtual int64_t ReceiveAll(void* out, int64_t outSize, int timeout_sec = 0, int timeout_usec = 100000) = 0;
        virtual bool Send(const void* data, int size) = 0;
        virtual bool Send(const std::string& data) = 0;
        virtual const char* GetLastError() const = 0;
        virtual const std::string& GetServerHost() const = 0;
        virtual void SetError(const std::string& error_msg) = 0;
        virtual bool IsClosed() const = 0;
	};

}
