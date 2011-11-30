#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

namespace http
{

/*struct header
{
    std::string name;
    std::string value;
};*/

std::map<std::string, std::string> ParseHeaders(std::vector<std::string>* vsHeaders)
{
	std::map<std::string, std::string> mHeaders;

    for (auto it = vsHeaders->begin(); it != vsHeaders->end(); it++)
	{
        boost::smatch m;
        bool match = boost::regex_match(*it, m, boost::regex("(.*): (.*)"));

        if (match)
        {
            /*header h;
            h.name = m[1];
            h.value = m[2];

            vhHeaders.push_back(h);*/

            mHeaders[m[1]] = m[2];
        }
	}

	return mHeaders;
}

}

#endif // HEADER_H_INCLUDED
