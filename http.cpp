#include "http.h"

Http_connection::Http_connection():
    Tcp_connection()
{
}

int Http_connection::Get(const std::string& path, std::string& result, int timeout_sec /*=DEF_TIMEOUT*/)
{
    std::string msg = 
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + m_server + "\r\n"
        "Accept: */*\r\n\r\n";

    int len = Send(msg);
    if (len == SOCKET_ERROR) {
        m_last_error = "Http: Cant send header. " + m_last_error;
        return len;
    }   

    std::string header;
    len = Receive_header(header, timeout_sec);
    if (len <= 0) {
		m_last_error = "Http: Cant receive header. " + m_last_error;
        return len;
    }

    int bodySize = Get_body_length(header);
    bool recive_to_end = false;
    if (bodySize <= 0) {
        m_last_error = "Http: Cant get body size ("+ header +").";
        recive_to_end = true;
    }

    len = recive_to_end? Receive(result, MAX_TCP_PACKET_SIZE, 0, 100000) : Receive(result, bodySize, timeout_sec);
    if (len <= 0) {
		m_last_error = "Http: Cant recive body. " + m_last_error;
        return len;
    }

    if (bodySize != len && !recive_to_end) {
        m_last_error = "Http: Wrong body size "+std::to_string(len)+" ("+ std::to_string(bodySize) +")";
        return SOCKET_ERROR;
    }

	if (Is_connection_closed(header)) {
		Close();
	}

    return len;
}

bool Http_connection::Is_closed()
{
	return !m_open;
}

int Http_connection::Receive_header(std::string& header, int timeout_sec /*= DEF_TIMEOUT*/)
{
    char buff;
    static const char end[4] = { '\r','\n','\r','\n' };
    int sz;
    int i = 0;

    do {
        sz = Receive(&buff, 1, timeout_sec);
        if (sz <= 0) { 
            return sz;
        }
        header += buff;
        if (buff == end[i]) { i++; }
        else { i = 0; }
    } while (i < 4);

    return (int)header.size();
}

int Http_connection::Get_body_length(const std::string& header)
{
    size_t pos = header.find("Content-Length");
    if (pos == std::string::npos) { return 0; }
    pos += 15;
    size_t posN = header.find("\r", pos);
    if (posN == std::string::npos) { return 0; }

    return static_cast<int>(stol(header.substr(pos, posN - pos)));
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
