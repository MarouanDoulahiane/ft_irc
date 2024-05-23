#include "../headers/IRCserv.hpp"
#include <string>
#include <cctype>

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void       trim(std::string& str) {
    size_t begin = 0;
    size_t end = str.size() -1;
    while (begin < end)
    {
        if (isspace(str[begin]))
            begin++;
        else if (isspace(str[end]))
            end--;
        else
            break;
    }
    if (begin != end)
        str = str.substr(begin, end - begin + 1);
    else
        str = "";
}


std::string getReasonForResponse(char *reason)
{
    if (!reason)
        return (std::string(""));
    std::string resp(" :");
    resp += std::string(reason);
    return resp;
}