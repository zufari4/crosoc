#pragma once

#include "crosoc.h"

class WSA_guard
{
public:
    static WSA_guard& instance()
    {
        static WSA_guard guard;
        return guard;
    }

    bool Init()
    {
        if (!m_init) {
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
                return false;
            }
            m_init = true;
        }
        return true;
    }
private:
    WSA_guard(): m_init(false) {}
    ~WSA_guard()
    {
        if (m_init) {
            WSACleanup();
        }
    }
    bool m_init;
};