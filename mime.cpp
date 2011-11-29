#include "mime.h"

std::map<std::string, std::string> MIME_TYPES;

std::string GetMimeType(boost::filesystem::path path)
{
	// Initialize MIME types
	if (MIME_TYPES.size() == 0);
		LoadMimeTypes();
	
	std::string extension = path.extension().string();
	// Strip the dot off the extension
	if (extension.size() > 0)
		extension = extension.substr(1);
	
	auto it = MIME_TYPES.find(extension);
	
	if (it != MIME_TYPES.end())
		return it->second;
	else
		return "text/plain";
}

// Load MIME types from resource file
void LoadMimeTypes()
{
	// Load mimetypes.txt
	std::string resource = GetFileResource(&_binary_mimetypes_txt_start, &_binary_mimetypes_txt_end);
	
	std::vector<std::string> mimetypes;
	// Split into rows
    boost::split_regex(mimetypes, resource, boost::regex("\\n"));
    
    // Fill the MIME_TYPES map
    for (auto it = mimetypes.begin(); it != mimetypes.end(); it++)
    {
    	boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("(.*)\\t(.*)"));
        if (match)
    		MIME_TYPES[m[1]] = m[2];	
    }
}
