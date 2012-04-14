#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <iostream>
#include <string>
#include <map>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "request.h"
#include "response.h"

namespace http
{

using boost::asio::ip::tcp;

class Server
{
public:
	void Construct();
	Server();
	Server(int port);
	~Server();
	// Bind the server to the specified port and start listening
	void Bind(int port);
	// Wait for a HTTP request
	http::Request* Recieve();
	// Reply with a HTTP response
	void Send(http::Response* response);
	
private:
	int m_Port;
	boost::asio::io_service m_IOService;
	tcp::acceptor* m_Acceptor;
	tcp::socket* m_Socket;
	tcp::endpoint m_Endpoint;
};

}

#endif
