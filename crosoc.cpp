#include "Crosoc.h"
#include "tcp.h"
#include "udp.h"
#include "http.h"

namespace Crosoc
{
    LIB_API std::unique_ptr<ITCPConnection> MakeTCPConnection()
    {
        auto res = std::make_unique<Tcp_connection>();
        return res;
    }

    LIB_API std::unique_ptr<IUDPConnection> MakeUDPConnection()
    {
        auto res = std::make_unique<Udp_connection>();
        return res;
    }

    LIB_API std::unique_ptr<IHTTPConnection> MakeHTTPConnection(bool useSSL /*= false*/)
    {
        auto res = std::make_unique<Http_connection>(useSSL);
        return res;
    }
}



