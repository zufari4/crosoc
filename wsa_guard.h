#pragma once

#include <atomic>

class WSA_guard
{
public:
    static WSA_guard& instance()
    {
        static WSA_guard guard;
        return guard;
    }
    bool Init();
private:
    WSA_guard();
    ~WSA_guard();
    std::atomic_bool m_init;
};