#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <boost/regex.hpp>

namespace http
{

// Creates a map of the headers in a raw HTTP request
static std::map<std::string, std::string> ParseHeaders(std::vector<std::string>* vsHeaders)
{
	std::map<std::string, std::string> mHeaders;

    for (auto it = vsHeaders->begin(); it != vsHeaders->end(); it++)
	{
        boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("(.*): (.*)"));

        if (match)
            mHeaders[m[1]] = m[2];
	}

	return mHeaders;
}

}

#endif // HEADER_H_INCLUDED
