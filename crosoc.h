#pragma once

#include "ITCPConnection.h"
#include "IUDPConnection.h"
#include "IHTTPConnection.h"
#include <memory>

#ifdef _WIN32
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __attribute__((visibility("default")))
#endif

namespace Crosoc
{
    LIB_API std::unique_ptr<Crosoc::ITCPConnection> MakeTCPConnection();
    LIB_API std::unique_ptr<Crosoc::ITCPConnection> MakeTCPConnection(const Endpoint& client);
    LIB_API std::unique_ptr<Crosoc::IUDPConnection> MakeUDPConnection();
    LIB_API std::unique_ptr<Crosoc::IHTTPConnection> MakeHTTPConnection(bool useSSL);
};
