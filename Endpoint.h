#pragma once

#include "net.h"
#include <inttypes.h>
#include <string>
#include <memory>

#ifdef _WIN32
#define LIB_API3 __declspec(dllexport)
#else
#define LIB_API3 __attribute__((visibility("default")))
#endif

namespace Crosoc
{
    struct Endpoint
    {
        SOCKET socket = INVALID_SOCKET;
        sockaddr_in addr = {0};
    };

    LIB_API3 void GetAddress(const Endpoint& enpoint, char* out, size_t outSize);
    LIB_API3 void Endpoint2Str(const Endpoint& enpoint, char* out, size_t outSize);
}