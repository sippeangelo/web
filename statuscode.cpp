#include "statuscode.h"

std::map<int, std::string> STATUS_CODES;

std::string GetResourceStatusString(int StatusCode)
{
	// Initialize statuscode strings from file
	// if it hasn't been loaded already.
	if (STATUS_CODES.size() == 0);
		LoadStatusCodes();
	
	// Find the requested status code in the map
	auto it = STATUS_CODES.find(StatusCode);
	
	// Return the corresponding string
	if (it != STATUS_CODES.end())
		return it->second;
	else
		return "Unknown";
}

// Load statuscode strings from resource file
void LoadStatusCodes()
{
	// Load statuscodes.txt from resources
	std::string resource = GetFileResource(&_binary_statuscodes_txt_start, &_binary_statuscodes_txt_end);
	
	std::vector<std::string> statuscodes;
	// Split into rows into a vector
    boost::split_regex(statuscodes, resource, boost::regex("\\R"));
    
    // Fill the STATUS_CODES map
    for (auto it = statuscodes.begin(); it != statuscodes.end(); it++)
    {
    	boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("^(.*?) (.*)$"));
        if (match)
        	STATUS_CODES[atoi(m[1].str().c_str())] = m[2].str();
    }
}
