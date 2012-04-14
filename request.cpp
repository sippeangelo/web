#include "request.h"

namespace http
{
	
Request::Request(std::string raw)
{
	std::vector<std::string> lines;
	// Split the raw data in to lines
	// "The escape sequence \R matches any line ending character sequence"
	boost::split_regex(lines, raw, boost::regex("\\R"));

	if (lines.size() > 0)
	{
		// Read the start line
		boost::smatch m;
		bool match = boost::regex_match(lines.at(0), m, boost::regex("^(.*) (.*) (.*)$"));
		if (match)		
			m_StartLine = { m[1].str(), m[2].str(), m[3].str() };
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

StartLine* Request::GetStartLine()
{
	return &m_StartLine;
}
	
}
