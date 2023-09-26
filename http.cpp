#include "http.h"
#include "tcp.h"
#include "ssl_conn.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>

static std::atomic_int counterID{0};

Http_connection::Http_connection(bool useSSL /*= false*/)
    : IHTTPConnection(useSSL)
    , useSSL_(useSSL)
{
    id_ = counterID++;
    m_conn = useSSL ? std::make_unique<Ssl_connection>() : std::make_unique<Tcp_connection>();
}

Http_connection::~Http_connection()
{
    Disconnect();
}

bool Http_connection::Connect(const std::string& server, int port /*= 80*/, int timeout_sec /*= DEF_TIMEOUT*/)
{
    return m_conn->Connect(server, port, timeout_sec);
}

void Http_connection::Disconnect()
{
    m_conn->Disconnect();
}

int64_t Http_connection::Get(const std::string& path, char* out, int64_t outSize, int timeout_sec /*=DEF_TIMEOUT*/)
{
    return Receive_data(Make_get_message(path), out, outSize, timeout_sec);
}

int64_t Http_connection::Post(const std::string& path, const Crosoc::HttpPostData& data, char* out, int64_t outSize, int timeout_sec /*= DEF_TIMEOUT*/)
{
    return Receive_data(Make_post_message(path, data), out, outSize, timeout_sec);
}
int64_t Http_connection::Receive_data(const std::string& message, char* out, int64_t outSize, int timeout_sec)
{
    int64_t len = m_conn->Send(message);
    if (len == SOCKET_ERROR) {
        return len;
    }

    std::string header;
    len = Receive_header(header, timeout_sec);
    if (len <= 0) {
        return len;
    }

    int64_t bodySize = Get_body_length(header);
    bool recive_to_end = false;
    bool is_chucked = false;
    if (bodySize <= 0) {
        m_conn->SetError("HTTP: Cant get body size (" + header + ")");
        is_chucked = Is_chunked(header);
        recive_to_end = !is_chucked;
    }

    if (is_chucked) {
        int64_t chuck_size;
        int64_t res;
        char buff[2];
        std::vector<uint8_t> result;

        do {
            chuck_size = 0;
            res = Recive_chunk_size(chuck_size, timeout_sec);
            if (res <= 0) {
                break;
            }
            if (chuck_size > 0) {
                res = static_cast<Tcp_connection*>(m_conn.get())->Receive(result, chuck_size, timeout_sec);
                if (res <= 0) {
                    break;
                }
            }
            // receive \r\n
            res = m_conn->Receive(&buff, 2, 2, timeout_sec);
        } while (res > 0 && chuck_size > 0);


        len = std::min((int64_t)result.size(), outSize - 1);
        if (len > 0) {
            out[len] = '\0';
            memcpy(out, result.data(), (size_t)len);
        }
        else {
            out[0] = '\0';
        }
    }
    else {
        len = recive_to_end ? m_conn->ReceiveAll(out, outSize) : m_conn->Receive(out, outSize, bodySize, timeout_sec);
        len = std::min(len, outSize - 1);
        if (len > 0) {
            out[len] = '\0';
        }
        else {
            out[0] = '\0';
        }
    }
    if (len <= 0) {
        return len;
    }

    if (bodySize != len && !recive_to_end && !is_chucked) {
        m_conn->SetError("HTTP: Wrong body size " + std::to_string(len) + " (" + std::to_string(bodySize) + ")");
        return SOCKET_ERROR;
    }

    if (Is_connection_closed(header)) {
        m_conn->Disconnect();
    }

    return len;
}


bool Http_connection::IsClosed() const
{
    return m_conn->IsClosed();
}


const char* Http_connection::GetLastError() const
{
    return m_conn->GetLastError();
}

int Http_connection::GetID() const
{
    return id_;
}

int64_t Http_connection::Receive_header(std::string& header, int timeout_sec /*= DEF_TIMEOUT*/)
{
    char buff;
    static const char end[4] = { '\r','\n','\r','\n' };
    int64_t sz;
    int i = 0;

    do {
        sz = m_conn->Receive(&buff, 1, 1, timeout_sec);
        if (sz <= 0) { 
            return sz;
        }
        header += buff;
        if (buff == end[i]) { i++; }
        else { i = 0; }
    } while (i < 4);

    return (int64_t)header.size();
}

int64_t Http_connection::Recive_chunk_size(int64_t& chunk_size, int timeout_sec /*= DEF_TIMEOUT*/)
{
    char buff;
    bool prevR = false;
    int64_t sz;
    int64_t all_size = 0;
    std::string strSize;

    do {
        sz = m_conn->Receive(&buff, 1, 1, timeout_sec);
        if (sz <= 0) {
            return sz;
        }
        all_size++;
        if (buff == '\n' && prevR) {
            break;
        }
        prevR = (buff == '\r');
        if (!prevR) {
            strSize += buff;
        }
    } while (sz > 0);

    chunk_size = (int64_t)strtoll(strSize.c_str(), NULL, 16);

    return all_size; 
}

int64_t Http_connection::Get_body_length(const std::string& header)
{
    size_t pos = header.find("Content-Length");
    if (pos == std::string::npos) { return 0; }
    pos += 15;
    size_t posN = header.find("\r", pos);
    if (posN == std::string::npos) { return 0; }

    return static_cast<int64_t>(stoll(header.substr(pos, posN - pos)));
}

bool Http_connection::Is_connection_closed(const std::string& header)
{
	size_t pos = header.find("Connection");
	if (pos == std::string::npos) { return false; }
	pos += 11;
	size_t posN = header.find("\r", pos);
	if (posN == std::string::npos) { return 0; }

	return header.substr(pos, posN - pos) == " close";
}

bool Http_connection::Is_chunked(const std::string& header) const
{
    return header.find("Transfer-Encoding: chunked") != std::string::npos;
}

std::string Http_connection::Make_get_message(const std::string& path) const
{
    return
    "GET " + path + " HTTP/1.1\r\n"
        "Host: " + m_conn->GetServerHost() + "\r\n"
        "Accept: */*\r\n\r\n";
}

std::string Http_connection::Make_post_message(const std::string& path, const Crosoc::HttpPostData& data) const
{
    std::string dataStr;
    for (const auto it : data)
    {
        if (!dataStr.empty()) {
            dataStr += '&';
        }
        dataStr += URLEncode(it.first) + "=" + URLEncode(it.second);
    }
    return
        "POST " + path + " HTTP/1.1\r\n"
        "Host: " + m_conn->GetServerHost() + "\r\n"
        "Content-Length: " + std::to_string(dataStr.size()) + "\r\n" +
        "Content-Type: application/x-www-form-urlencoded\r\n" +
        "Accept: */*\r\n\r\n" +
        dataStr;
}

std::string Http_connection::URLEncode(const std::string& s)
{
    std::string enc;
    const size_t len = s.length();
    char buf[3];
    buf[0] = '%';
    for (size_t i = 0; i < len; i++)
    {
        const unsigned char c = s[i];
        // from  https://www.ietf.org/rfc/rfc1738.txt, page 3
        // with some changes for compatibility
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == ',')
            enc += (char)c;
        else if (c == ' ')
            enc += '+';
        else
        {
            unsigned nib = (c >> 4) & 0xf;
            buf[1] = (char)(nib < 10 ? '0' + nib : 'a' + (nib - 10));
            nib = c & 0xf;
            buf[2] = (char)(nib < 10 ? '0' + nib : 'a' + (nib - 10));
            enc.append(&buf[0], 3);
        }
    }
    return enc;
}