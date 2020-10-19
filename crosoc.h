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
    LIB_API std::unique_ptr<ITCPConnection> MakeTCPConnection();
    LIB_API std::unique_ptr<IUDPConnection> MakeUDPConnection();
    LIB_API std::unique_ptr<IHTTPConnection> MakeHTTPConnection(bool useSSL);
}
