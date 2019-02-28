#include "crosoc.h"
#ifdef _WIN32
#include "wsa_guard.h"
#else
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace crosoc
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
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
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
            int status = 0;
    #ifdef _WIN32
            status = shutdown(sock, SD_BOTH);
            if (status == 0) { closesocket(sock); }
    #else
            status = shutdown(sock, SHUT_RDWR);
             if (status == 0) { close(sock); }
    #endif
            sock = INVALID_SOCKET;
        }
    }

    bool Make_endpoint(const std::string& host, int port, sockaddr_in& res)
    {
        unsigned long addr = INADDR_NONE;
        if (host.empty()) {
            addr = htonl(INADDR_ANY);
        }
        else {
            inet_pton(AF_INET, host.c_str(), &addr);
        }

        if (addr == INADDR_NONE) {
            struct addrinfo  hints = { 0 };
            struct addrinfo* result = nullptr;
            hints.ai_family   = AF_UNSPEC;
            hints.ai_socktype = SOCK_DGRAM;

            if (getaddrinfo(host.c_str(), "13", &hints, &result) != 0) {
                return false;
            }
            if (result == nullptr) {
                return false;
            }

            for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
                if (rp->ai_addr) {
                    addr = ((sockaddr_in*)rp->ai_addr)->sin_addr.s_addr;
                    break;
                }
            }

			freeaddrinfo(result);
            if (addr == INADDR_NONE) {
                return false;
            }
        }

        memset(&res, 0, sizeof(res));
        res.sin_family = AF_INET;
        res.sin_addr.s_addr = addr;
        res.sin_port = htons((uint16_t)port);
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
