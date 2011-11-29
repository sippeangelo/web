#include <iostream>
#include <string>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <unistd.h>

#include "resources.h"
#include "mime.h"

using boost::asio::ip::tcp;

struct header
{
    std::string name;
    std::string value;
};

std::map<std::string, std::string> ParseHeaders(std::vector<std::string>* vsHeaders)
{
	std::vector<header> vhHeaders;

	std::map<std::string, std::string> mHeaders;

    for (auto it = vsHeaders->begin(); it != vsHeaders->end(); it++)
	{
        boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("(.*): (.*)"));

        if (match)
        {
            header h;
            h.name = m[1];
            h.value = m[2];

            vhHeaders.push_back(h);

            mHeaders[m[1]] = m[2];
        }
	}

	return mHeaders;
}

int main(int argc, char **argv)
{
	int bind_port = 1024;
	
	if (geteuid() != 0)
	{
		std::cout << "Needs to be run as root" << std::endl;
		return 1;	
	}
	
	// Chroot to working directory
	chroot(".");
	
    try
    {
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), bind_port));
		
		for (;;)
		{
			tcp::socket socket(io_service);
			tcp::endpoint endpoint;
			acceptor.accept(socket, endpoint);

			std::cout << "Connected: " << endpoint.address() << std::endl;

            std::vector<std::string> headers;

            try
            {
                boost::array<char, 2048> buf;
                size_t len = socket.read_some(boost::asio::buffer(buf));
                std::string request = std::string(buf.data(), len);
                boost::split_regex(headers, request, boost::regex("\\n"));
            }
            catch (std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }

            if (headers.size() > 0)
            {
                boost::smatch m;
                boost::regex_match(headers.at(0), m, boost::regex(".* (.*) .*"));
                std::string path = m[1].str();
                std::cout << path << std::endl;
				
                std::map<std::string, std::string> hHeaders = ParseHeaders(&headers);

				//std::cout << "___ REQUEST ___" << std::endl;
                //for (auto it = hHeaders.begin(); it != hHeaders.end(); it++)
				//	std::cout << it->first << " = " << it->second << std::endl;

				boost::filesystem::path p(path);

                std::stringstream message;
                if (boost::filesystem::exists(p))
					message << "You requested " << path;
				else
					message << "404";
					
				// 404
				if (!boost::filesystem::exists(p))
				{
					std::string html = GetFileResource(&_binary_404_html_start, &_binary_404_html_end);
					
					std::stringstream ss;
					ss << "HTTP/1.1 404 Not Found\n";
					//ss << "Content-Type: text/html\n";
					ss << "Content-Length: " << html.size() << "\n";
					ss << "\n";
					ss << html;

					std::string response = ss.str();

					std::cout << "___ RESPONSE ___" << std::endl << ss.str() << std::endl;
					
					// Send headers
					boost::asio::write(socket, boost::asio::buffer(response), boost::asio::transfer_all());										
				}
				// File
				else if (boost::filesystem::is_regular_file(p))
				{
					int file_size = boost::filesystem::file_size(p);
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
					boost::asio::write(socket, boost::asio::buffer(buffer, file_size), boost::asio::transfer_all());
				}
				// Directory listing
				else if (boost::filesystem::is_directory(p))
				{
					std::stringstream html;
					
					html << "<h1>" << p.string() << "</h1>";
					
					for (auto it = boost::filesystem::directory_iterator(p); it != boost::filesystem::directory_iterator(); it++)
						html << "<a href=\"" << (*it).path().string() << "\">" << (*it).path().string() << "</a><br>";
							
					std::stringstream ss;
					ss << "HTTP/1.1 200 OK\n";
					ss << "Content-Type: text/html\n";
					ss << "Content-Length: " << html.str().length() << "\n";
					ss << "\n";
					ss << html.str();				
																				
					std::string response = ss.str();
					std::cout << "___ RESPONSE ___" << std::endl << ss.str() << std::endl;
					boost::asio::write(socket, boost::asio::buffer(response), boost::asio::transfer_all());
				}
            }

            std::cout << "Disconnected: " << endpoint.address() << std::endl;

            socket.close();
		}

    }
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
