#ifndef STATUSCODE_H_INCLUDED
#define STATUSCODE_H_INCLUDED

#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>

#include "resources.h"

extern std::map<int, std::string> STATUS_CODES;

// Translates a status code into a response string
std::string GetResourceStatusString(int StatusCode);
void LoadStatusCodes();

#endif // STATUSCODE_H_INCLUDED
