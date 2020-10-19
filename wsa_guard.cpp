#include "net.h"
#include "wsa_guard.h"

bool WSA_guard::Init()
{
    if (!m_init) {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            return false;
        }
#endif
        m_init = true;
    }
    return true;
}

WSA_guard::WSA_guard() 
    : m_init(false)
{
}

WSA_guard::~WSA_guard()
{
#ifdef _WIN32
    if (m_init) {
        WSACleanup();
    }
#endif
}
