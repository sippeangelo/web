#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED 

#include <string>

extern "C"
{
	// mimetypes.txt
	extern char _binary_mimetypes_txt_start;
	extern char _binary_mimetypes_txt_end;		
	
	// statuscodes.txt
	extern char _binary_statuscodes_txt_start;
	extern char _binary_statuscodes_txt_end;
	
	// 403.html
	extern char _binary_403_html_start;
	extern char _binary_403_html_end;
		
	// 404.html
	extern char _binary_404_html_start;
	extern char _binary_404_html_end;
}

// Get a file embedded into the executable
// TODO: Allow overriding with real files
inline std::string GetFileResource(char* start, char* end)
{
	// Create a character buffer with the size 
	// of the memory block.
	int size = (int)(end - start) / sizeof(char);
	char* buffer = new char[size];
	
	// Fill the buffer
	for (char* p = start; p != end; p++)
		buffer[(int)(p - start)] = *p;
	
	// Convert to string
	std::string file(buffer, (size_t)size);
	
	delete[] buffer;
	
	return file;
}

#endif
