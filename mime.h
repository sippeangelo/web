#ifndef MIME_H_INCLUDED
#define MIME_H_INCLUDED

#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>

#include "resources.h"

extern std::map<std::string, std::string> MIME_TYPES;

std::string GetMimeType(boost::filesystem::path path);
void LoadMimeTypes();

#endif // MIME_H_INCLUDED
