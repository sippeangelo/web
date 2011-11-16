#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

using boost::asio::ip::tcp;

int main()
{
    try
    {
		boost::asio::io_service io_service;
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 80));

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

                std::string data = std::string(buf.data(), len);
                boost::split_regex(headers, data, boost::regex("\\n"));
            }
            catch (std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }

            if (headers.size() > 0)
            {
                //for (int i = 0; i < headers.size() - 1; i++)
                //    std::cout << headers.at(i) << std::endl;

                boost::smatch m;
                boost::regex_match(headers.at(0), m, boost::regex(".* (.*) .*"));
                std::cout << m[1] << std::endl;

                boost::asio::write(socket, boost::asio::buffer("HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 5\n\nHello"),
                    boost::asio::transfer_all());
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
