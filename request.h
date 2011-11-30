#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

#include "header.h"

namespace http
{

struct StartLine
{
	std::string Version;
	std::string Resource;
	std::string Method;
};

class Request
{
public:
	Request(std::string raw);
  	~Request();
  	
  	std::string GetHeader(std::string name);
  	std::map<std::string, std::string>* GetHeaders();
  	struct StartLine GetStartLine();
  	
private:
  	struct StartLine m_StartLine;
  	std::map<std::string, std::string> m_Headers;
};

Request::Request(std::string raw)
{
	std::vector<std::string> lines;
	// Split the raw data in to lines
	boost::split_regex(lines, raw, boost::regex("\\n"));

	if (lines.size() > 0)
	{
		// Read the start line
		boost::smatch m;
		bool match = boost::regex_match(lines.at(0), m, boost::regex("(.*) (.*) (.*)"));
		if (match)
		{
			m_StartLine = { m[1].str(), m[2].str(), m[3].str() };
		}
		else
		{
			std::cerr << "Failed to read request start line" << std::endl;
			return;	
		}

		// Parse ALL the headers!
		m_Headers = ParseHeaders(&lines);
	}
}

std::string Request::GetHeader(std::string name)
{
	auto it = m_Headers.find(name);
	
	if (it != m_Headers.end())
		return it->second;
	else
		return "";
}

std::map<std::string, std::string>* Request::GetHeaders()
{
	return &m_Headers;
}

struct StartLine Request::GetStartLine()
{
	return m_StartLine;
}

}

#endif // REQUEST_H_INCLUDED
