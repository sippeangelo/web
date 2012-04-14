#include "response.h"

namespace http
{

Response::Response(std::string HTTPVersion, int statusCode)
{
	m_HTTPVersion = HTTPVersion;
	m_StatusCode = statusCode;
	m_StatusString = GetResourceStatusString(statusCode);
	
	m_Content = NULL;
}

Response::~Response()
{
	if (m_Content != NULL)
		delete m_Content;
}

void Response::SetHeaders(std::map<std::string, std::string>* headers)
{
	m_Headers = *headers;
}
std::map<std::string, std::string>* Response::GetHeaders()
{
	return &m_Headers;	
}

void Response::SetHeader(std::string header, std::string value)
{
	m_Headers[header] = value;	
}

void Response::DeleteHeader(std::string header)
{
	m_Headers.erase(header);	
}

void Response::SetContent(std::string* content)
{
	m_Content = content;
}

std::string* Response::GetContent()
{
	return m_Content;	
}

}
