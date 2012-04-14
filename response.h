#ifndef RESPONSE_H_INCLUDED
#define RESPONSE_H_INCLUDED

#include <iostream>
#include <string>
#include <map>
#include "statuscode.h"

namespace http
{

// The structure of a HTTP response
class Response 
{
public:
	Response(std::string HTTPVersion, int StatusCode);
	~Response();
	
	std::string GetVersion() { return m_HTTPVersion; }
	int GetStatusCode() { return m_StatusCode; }
	std::string GetStatusString() { return m_StatusString; }
	
	// Sets the complete internal header map to the one passed with the function
	void SetHeaders(std::map<std::string, std::string>* headers);
	std::map<std::string, std::string>* GetHeaders();
	// Sets a specific header without touching the rest
	void SetHeader(std::string header, std::string value);
	// Delete a specific header without touching the rest
	void DeleteHeader(std::string header);
	
	// Set the content of the response
	void SetContent(std::string* content);
	std::string* GetContent();
	
private:
	std::string m_HTTPVersion;
	int m_StatusCode;
	std::string m_StatusString;

	std::map<std::string, std::string> m_Headers;

	std::string* m_Content;
};

	
}

#endif
