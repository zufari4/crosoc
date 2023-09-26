#include "Endpoint.h"
#include <cstring>

namespace Crosoc
{
    size_t strlcpy(char* dst, const std::string& src, size_t maxlen)
    {
        const size_t srclen = src.length();
        size_t copyLen;
        if (srclen + 1 < maxlen) {
            copyLen = srclen + 1;
            memcpy(dst, src.c_str(), copyLen);
        }
        else if (maxlen != 0) {
            copyLen = maxlen - 1;
            memcpy(dst, src.c_str(), copyLen);
            dst[copyLen] = '\0';
        }
        else {
            copyLen = 0;
        }
        return copyLen;
    }

    void GetAddress(const Endpoint& enpoint, char* out, size_t outSize)
    {
        uint32_t ip = enpoint.addr.sin_addr.s_addr;
        std::string res = std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[0])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[1])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[2])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[3])) + ':' +
            std::to_string(enpoint.addr.sin_port);
        strlcpy(out, res, outSize);
    }

    void Endpoint2Str(const Endpoint& enpoint, char* out, size_t outSize) 
    {
        uint32_t ip = enpoint.addr.sin_addr.s_addr;
        std::string res = std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[0])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[1])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[2])) + '.' +
            std::to_string(uint8_t(reinterpret_cast<char*>(&ip)[3]));
        strlcpy(out, res, outSize);
    }

    }