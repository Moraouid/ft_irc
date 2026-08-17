#ifndef IRC_UTILS_HPP
#define IRC_UTILS_HPP

#include <string>
#include <vector>

std::vector<std::string> splitString(const std::string &input, const std::string &delimiter);
std::string generateTimestamp();
void debugPrint(const std::string &message);
std::string formatReply(const std::string &code, const std::string &nickname, const std::string &message);

#endif
