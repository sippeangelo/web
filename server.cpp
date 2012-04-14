#include "server.h"

namespace http
{

void Server::Construct()
{
	m_Acceptor = NULL;
	m_Socket = NULL;
}

Server::Server()
{
	Construct();	
}

Server::Server(int port)
{
	Construct();
	this->Bind(port);	
}

Server::~Server()
{
	if (m_Acceptor != NULL)
		delete m_Acceptor;	
	if (m_Socket != NULL)
		delete m_Socket;
}

void Server::Bind(int port)
{
	m_Port = port;
	m_Acceptor = new tcp::acceptor(m_IOService, tcp::endpoint(tcp::v4(), port));
	m_Socket = new tcp::socket(m_IOService);;
}

http::Request* Server::Recieve()
{
	// Accept the connection, blocking
	m_Acceptor->accept(*m_Socket, m_Endpoint);
	
	http::Request* req = NULL;
	
	try
	{
		boost::array<char, 2048> buf;
		// Read the raw HTTP request
		size_t len = m_Socket->read_some(boost::asio::buffer(buf));
		std::string request = std::string(buf.data(), len);

		// Create a HTTP request object and return it
		req = new http::Request(request);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return req;
}

void Server::Send(http::Response* response)
{
	// Create the response line (Version StatusCode StatusString)
	std::stringstream ss;
	ss << response->GetVersion() << " " << response->GetStatusCode() << " " << response->GetStatusString() << "\r\n";
	
	// Append all the headers
	std::map<std::string, std::string>* headers = response->GetHeaders();
	for (auto it = headers->begin(); it != headers->end(); it++)
		ss << (*it).first << ": " << (*it).second << "\r\n";
	ss << "\r\n";
	
	try
	{
		// Send headers
		boost::asio::write(*m_Socket, boost::asio::buffer(ss.str()), boost::asio::transfer_all());
		std::cout << ss.str();
		// Send file data
		if (response->GetContent() != NULL)
			boost::asio::write(*m_Socket, boost::asio::buffer(*response->GetContent()), boost::asio::transfer_all());
		//else
		//	std::cout << "No content" << std::endl;
	}
 	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	m_Socket->close();
		
/*	struct http::Response::StartLine* startLine = response->GetStartLine();
	std::stringstream ssStartLine;
	ssStartLine<< startLine->Version << " " << startLine->ResponseCode << " " << startLine->ResponseString;
	
	std::map<std::string, std::string>* headers = response->GetHeaders();
	std::stringstream ssHeaders;
	for (auto it = headers->begin(); it != headers->end(); it++)
		ssHeaders << (*it).first << ": " << (*it).second << "\n";
	ssHeaders << "\n";
	
	char* content = response->GetContent();
	
	// Send start line
	boost::asio::write(*m_Socket, boost::asio::buffer(ssStartLine.str()), boost::asio::transfer_all());
	// Send headers
	boost::asio::write(*m_Socket, boost::asio::buffer(ssHeaders.str()), boost::asio::transfer_all());
	// Send file data
	boost::asio::write(*m_Socket, boost::asio::buffer(content, 7), boost::asio::transfer_all());
	
	
	m_Socket->close();*/
	/*int file_size = boost::filesystem::file_size(p);
	boost::filesystem::ifstream file(p, std::ios::binary);
	char* buffer = new char[file_size];
	file.read(buffer, file_size);

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\n";
	ss << "Content-Type: " << GetMimeType(p) << "\n";
	ss << "Content-Length: " << file_size << "\n";
	ss << "\n";
	//ss << buffer;

	std::string headers = ss.str();

	std::cout << "___ RESPONSE ___" << std::endl << ss.str() << std::endl;
	
	// Send headers
	boost::asio::write(socket, boost::asio::buffer(headers), boost::asio::transfer_all());
	// Send file data
	boost::asio::write(socket, boost::asio::buffer(buffer, file_size), boost::asio::transfer_all());*/
	
}

}
