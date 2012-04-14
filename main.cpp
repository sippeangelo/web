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
#include <openssl/md5.h>
#include <stdio.h>

#include "server.h"
#include "resources.h"
#include "mime.h"
#include "header.h"
#include "request.h"
#include "response.h"

using boost::asio::ip::tcp;

std::map<std::string, std::string> ParseHeaders(std::vector<std::string>* vsHeaders)
{
	std::map<std::string, std::string> mHeaders;

    for (auto it = vsHeaders->begin(); it != vsHeaders->end(); it++)
	{
        boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("(.*): (.*)"));

        if (match)
        {
            /*header h;
            h.name = m[1];
            h.value = m[2];

            vhHeaders.push_back(h);*/

            mHeaders[m[1]] = m[2];
        }
	}

	return mHeaders;
}

int main(int argc, char **argv)
{
	// Create a server object and bind to port 8080
	http::Server* srv = new http::Server();
	srv->Bind(8080);

	// There's still no way to escape this loop
	while (true)
	{
		// Listen of connections
		http::Request* req = srv->Recieve();

		if (req != NULL)
		{
			// Get the start line and print it out for debugging purposes
			// ... No, really just for fun.
			http::StartLine* sl = req->GetStartLine();
			std::cout << sl->Version << " " << sl->Method << " " << sl->Resource << std::endl;

			// Create a response structure we'll fill out
			http::Response* resp = NULL;

			// The raw resource path from the request
			std::string requestPath = req->GetStartLine()->Resource;
			// Relative path, from the query
			boost::filesystem::path relativePath = boost::filesystem::path(requestPath).relative_path();
			// Absolute filesystem path
			boost::filesystem::path absolutePath = boost::filesystem::absolute(relativePath);

			// If the requested resource is a directory, serve the index
			if (boost::filesystem::is_directory(absolutePath))
			{
				// Iterate through the directory, searching for a file with the filename "index"
				for (auto it = boost::filesystem::directory_iterator(absolutePath); it != boost::filesystem::directory_iterator(); it++)
				{
					// Only search for actual files, not directories
					if (boost::filesystem::is_regular_file((*it).path()))
					{
						boost::filesystem::path filename = (*it).path().filename();
						if (filename.stem() == "index")
						{
							// Append the index filename to the absolute path
							absolutePath /= filename;
							break;
						}
					}
				}
			}

			boost::filesystem::path path = absolutePath; // For convinience

			if (boost::filesystem::exists(path))
			{
				// Deny access to files outside the document root and to files we don't have read access
				if (path < boost::filesystem::current_path() || access(path.c_str(), R_OK) != 0)
				{
					// Send a 403 Forbidden
					resp = new http::Response("HTTP/1.1", 403);
					resp->SetHeader("Content-Type", "text/html");

					// Load 403.html
					std::string* html = new std::string(GetFileResource(&_binary_403_html_start, &_binary_403_html_end));
					resp->SetContent(html);
				}
				// If we have requested a file, send it
				else if (boost::filesystem::is_regular_file(path))
				{
					// Read the file
					int file_size = boost::filesystem::file_size(path);
					boost::filesystem::ifstream file;
					file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
					file.open(path, std::ios::binary);
					char* buffer = new char[file_size];
					file.read(buffer, file_size);

					// Get file stats
					struct stat sb;
					stat(absolutePath.string().c_str(), &sb);
					// Compose an ETag consisting of file inode, file size and 
					// modification timestamp like how the big boys at Apache do.
					std::stringstream ssETag;
					ssETag << "\"" << std::hex << sb.st_ino << "-" << std::hex << sb.st_size << "-" << std::hex << sb.st_mtime << "\"";
					std::string ETag = ssETag.str();

    				std::cout << "None match: " << req->GetHeader("If-None-Match") << std::endl;
    				// Compare the ETag
    				if (req->GetHeader("If-None-Match") == ETag)
    				{
    					// The ETag matches, send a 304 Not Modified
						resp = new http::Response("HTTP/1.1", 304);
    				}
    				else
    				{
						// Send the file along with a 200 OK
						resp = new http::Response("HTTP/1.1", 200);
						// Set the content type based on file extension
						resp->SetHeader("Content-Type", GetMimeType(path));
						// Set the ETag header
						resp->SetHeader("ETag", ETag);
						
						// Set the file contents
						resp->SetContent(new std::string(buffer, file_size));
    				}
    				
    				delete[] buffer;
				}
				// If the index search failed to find an index file, serve a directory listing
				else if (boost::filesystem::is_directory(path))
				{
					// For web server sanity, we require a trailing slash after directory
					// names to denote that the resource is a directory and not a file.
					if (*(--requestPath.end()) == '/') // Iterator to last character in the string
					{
						// Put together a directory listing
						resp = new http::Response("HTTP/1.1", 200);
						resp->SetHeader("Content-Type", "text/html");

						std::string html;
						html += "<h1>Index of /" + relativePath.string() + "</h1>";
						html += "<a href=\"..\">..</a><br>";

						// Loop through the directory and add links to all resources
						for (auto it = boost::filesystem::directory_iterator(absolutePath); it != boost::filesystem::directory_iterator(); it++)
							html += "<a href=\"" + (*it).path().filename().string() + "\">" + (*it).path().filename().string() + "</a><br>";

						resp->SetContent(new std::string(html));
					}
					// If there is no slash; send a 301 Moved Permanently to help the browser out.
					else
					{
						resp = new http::Response("HTTP/1.1", 301);
						resp->SetHeader("Location", requestPath + "/");
					}
				}
			}
			else
			{
				// Send 404 File Not Found
				resp = new http::Response("HTTP/1.1", 404);
				resp->SetHeader("Content-Type", "text/html");

				// Load 404.html
				std::string* html = new std::string(GetFileResource(&_binary_404_html_start, &_binary_404_html_end));
				resp->SetContent(html);
			}

			// No page served for some reason, 500 Internal Server Error
			if (resp == NULL)
			{
				resp = new http::Response("HTTP/1.1", 500);
			}

			// Append some general headers
			resp->SetHeader("Server", "imon/1");


			srv->Send(resp);

			delete resp;
		}
		else
		{
			std::cerr << "Recieve failed" << std::endl;
		}

		delete req;
	}

	delete srv;

	/*pid_t pID = fork();

	std::cout << "PID: " << pID << std::endl;

	if (pID == 0)
	{
		std::cout << "Lol fork" << std::endl;
	}
	else if (pID < 0)
	{
		std::cerr << "Failed to fork D:" << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "Yey we did the fork!" << std::endl;
	}

	std::cin.get();
	return 0;*/


	/*int bind_port = 8080;

	if (geteuid() != 0 && bind_port < 1024)
	{
		std::cout << "Needs to be run as root to bind ports below 1024" << std::endl;
		return 1;
	}

	// Chroot to working directory
	if (chroot("."))
	{
		std::cout << "chroot failed" << std::endl;
		//return 1;
	}

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


				boost::filesystem::path working("/home/imon/cpp/web/bin/Debug");
				boost::filesystem::path p(path);

				if (p < working)
				{
					std::cout << "OMG HAX!" << std::endl;
				}

				// 403
				if (p < working && false)
				{
					std::string html = GetFileResource(&_binary_403_html_start, &_binary_403_html_end);

					std::stringstream ss;
					ss << "HTTP/1.1 403 Forbidden\n";
					//ss << "Content-Type: text/html\n";
					ss << "Content-Length: " << html.size() << "\n";
					ss << "\n";
					ss << html;

					std::string response = ss.str();

					std::cout << "___ RESPONSE ___" << std::endl << ss.str() << std::endl;

					// Send headers
					boost::asio::write(socket, boost::asio::buffer(response), boost::asio::transfer_all());
				}
				// 404
				else if (!boost::filesystem::exists(p))
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
	}*/

	return 0;
}
