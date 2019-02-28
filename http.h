#pragma once
#include "tcp.h"

class Http_connection: public Tcp_connection
{
public:
    Http_connection();
    virtual ~Http_connection() {};
    virtual int Get(const std::string& path, std::string& result, int timeout_sec=DEF_TIMEOUT);
	virtual bool Is_closed();
protected:
    int  Receive_header(std::string& header, int timeout_sec = DEF_TIMEOUT);
    int  Get_body_length(const std::string& header);
	bool Is_connection_closed(const std::string& header);
};