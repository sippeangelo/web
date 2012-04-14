#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include "header.h"

namespace http
{

// A HTTP request line consisting of a Version, Method and requested Resource
struct StartLine
{
	std::string Method;
	std::string Resource;
	std::string Version;
};

// Abscracts a raw HTTP request into a http::StartLine and a map of headers
class Request
{
public:
	Request(std::string raw);
  	~Request() {}
  	
  	std::string GetHeader(std::string name);
  	std::map<std::string, std::string>* GetHeaders();
  	StartLine* GetStartLine();
  	
private:
  	StartLine m_StartLine;
  	std::map<std::string, std::string> m_Headers;
};

}

#endif // REQUEST_H_INCLUDED
