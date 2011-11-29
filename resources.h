#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED 

#include <string>

extern "C"
{
	// mimetypes.txt
	extern char _binary_mimetypes_txt_start;
	extern char _binary_mimetypes_txt_end;	
	
	// 404.html
	extern char _binary_404_html_start;
	extern char _binary_404_html_end;
}

// TODO: Allow overriding
inline std::string GetFileResource(char* start, char* end)
{
	char* buffer = new char[(int)(end - start) / sizeof(char)];
		
	for (char* p = start; p != end; p++)
		buffer[(int)(p - start)] = *p;
	
	return std::string(buffer);
}

#endif
