#include "irc_utils.hpp"

#include <ctime>
#include <iostream>

void debugPrint(const std::string &message)
{
    std::cout << "[DEBUG] " << message << std::endl;
}

std::vector<std::string> splitString(const std::string &input, const std::string &delimiter)
{
    std::vector<std::string> parts;
    std::string copy = input;
    std::string::size_type pos = 0;

    while ((pos = copy.find(delimiter)) != std::string::npos)
    {
        std::string token = copy.substr(0, pos);
        if (!token.empty())
            parts.push_back(token);
        copy.erase(0, pos + delimiter.length());
    }
    if (!copy.empty())
        parts.push_back(copy);
    return parts;
}

std::string generateTimestamp()
{
    char buffer[9];
    time_t rawTime = time(NULL);
    struct tm *timeInfo = localtime(&rawTime);

    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeInfo);
    return std::string(buffer);
}
