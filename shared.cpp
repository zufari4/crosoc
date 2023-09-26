#include "shared.h"
#ifdef _WIN32
#include "wsa_guard.h"
#else
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include <memory>

namespace Crosoc
{
    bool Init()
    {
    #ifdef _WIN32
        WSA_guard::instance().Init();
    #endif
        return true;
    }

    std::string Get_last_error()
    {
        char msgbuf[256];
        msgbuf [0] = '\0';

    #ifdef _WIN32
        int err = WSAGetLastError();
        FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
               NULL,                // lpsource
               err,                 // message id
               MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
               msgbuf,              // output buffer
               sizeof (msgbuf),     // size of msgbuf, bytes
               NULL);               // va_list of arguments
    #else
        strerror_r(errno, msgbuf, sizeof (msgbuf));
    #endif

        return msgbuf;
    }

    void Close(SOCKET& sock)
    {
        if (sock != INVALID_SOCKET) {
    #ifdef _WIN32
            shutdown(sock, SD_BOTH);
            closesocket(sock);
    #else
            shutdown(sock, SHUT_RDWR);
            close(sock);
    #endif
            sock = INVALID_SOCKET;
        }
    }

    void FillByAddr(sockaddr_in& res, unsigned long addr, int port)
    {
        memset(&res, 0, sizeof(sockaddr_in));
        res.sin_family      = AF_INET;
        res.sin_addr.s_addr = addr;
        res.sin_port        = htons(static_cast<uint16_t>(port));
    }

    bool Make_endpoint(const std::string& host, int port, sockaddr_in& res, ProtocolType protoType /*= ProtocolType::TCP*/)
    {
        if (host.empty()) {
            FillByAddr(res, INADDR_ANY,  port);
            return true;
        }

        // Try to convert from IP address
        {
            unsigned long addr = INADDR_NONE;
            inet_pton(AF_INET, host.c_str(), &addr);
            if (addr != INADDR_NONE) {
                FillByAddr(res, addr, port);
                return true;
            }
        }

        // Try to get addr from host name
        {
            addrinfo* add_list = nullptr;
            addrinfo  hints;
            hints.ai_flags = 0;
            hints.ai_family = AF_INET;
            hints.ai_addrlen = 0;
            hints.ai_canonname = nullptr;
            hints.ai_addr = nullptr;
            hints.ai_next = nullptr;
            if (protoType == ProtocolType::TCP) {
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_protocol = IPPROTO_TCP;
            }
            else if (protoType == ProtocolType::UDP) {
                hints.ai_socktype = SOCK_DGRAM;
                hints.ai_protocol = IPPROTO_UDP;
            }
            else {
                hints.ai_socktype = 0;
                hints.ai_protocol = 0;
            }

            if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &add_list) != 0) {
                return false;
            }
            if (add_list == nullptr) {
                return false;
            }

            res = *((sockaddr_in*)(add_list->ai_addr));
            freeaddrinfo(add_list);
        }

        return true;
    }

    std::string Get_host(const sockaddr_in& addr)
    {
        char buff[NI_MAXHOST];
        unsigned long tmp_addr = addr.sin_addr.s_addr;
        if (inet_ntop(AF_INET, &tmp_addr, buff, NI_MAXHOST)) {
            return buff;
        }
        else {
            return std::string();
        }
    }

    int Get_port(const sockaddr_in& addr)
    {
        return ntohs(addr.sin_port);
    }

    void Set_nonblocking(SOCKET sock, bool nonblocking)
    {
    #ifdef _WIN32
        auto flags = nonblocking ? 1UL : 0UL;
        ioctlsocket(sock, FIONBIO, &flags);
    #else
        auto flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, nonblocking ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK)));
    #endif
    }

    bool Is_connection_error()
    {
    #ifdef _WIN32
        return WSAGetLastError() != WSAEWOULDBLOCK;
    #else
        return errno != EINPROGRESS;
    #endif
    }

    bool Wait_socket_is_ready(SOCKET sock, long sec, long usec /*=0*/)
    {
        fd_set fdsr;
        FD_ZERO(&fdsr);
        FD_SET(sock, &fdsr);

        auto fdsw = fdsr;
        auto fdse = fdsr;

        timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;

        if (select((int)sock + 1, &fdsr, &fdsw, &fdse, &tv) < 0) {
            return false;
        } else if (FD_ISSET(sock, &fdsr) || FD_ISSET(sock, &fdsw)) {
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0 || error) {
                return false;
            }
        } else {
            return false;
        }

        return true;
    }

    int Wait_data(SOCKET sock, long sec, long usec/*=0*/)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;

        return select((int)sock + 1, &fds, NULL, NULL, &tv);
    }
}
