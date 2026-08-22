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

std::string formatReply(const std::string &code, const std::string &nickname, const std::string &message)
{
    return std::string(":irc ") + code + " " + nickname + " " + message + "\r\n";
}

std::string formatPrivmsg(const std::string &nickname, const std::string &username, const std::string &target, const std::string &text)
{
    return ":" + nickname + "!" + username + "@localhost PRIVMSG " + target + " :" + text + "\r\n";
}

std::string formatNotice(const std::string &target, const std::string &message)
{
    return ":irc NOTICE " + target + " :" + message + "\r\n";
}

std::string rplWelcome(const std::string &serverName, const std::string &nickname)
{
    return ":" + serverName + " 001 " + nickname + " :Welcome to the Internet Relay Network, " + nickname + "\r\n";
}

std::string rplTopic(const std::string &serverName, const std::string &nickname, const std::string &channel, const std::string &topic)
{
    return ":" + serverName + " 332 " + nickname + " " + channel + " :" + topic + "\r\n";
}

std::string rplInviting(const std::string &serverName, const std::string &nickname, const std::string &channel, const std::string &targetNick)
{
    return ":" + serverName + " 341 " + nickname + " " + targetNick + " " + channel + "\r\n";
}

std::string inviteMsg(const std::string &nickname, const std::string &username, const std::string &ipAddress, const std::string &targetNick, const std::string &channelName)
{
    return ":" + nickname + "!" + username + "@" + ipAddress + " INVITE " + targetNick + " :" + channelName + "\r\n";
}

std::string rplJoin(const std::string &nickname, const std::string &username, const std::string &ipAddress, const std::string &channelName)
{
    return ":" + nickname + "!" + username + "@" + ipAddress + " JOIN :" + channelName + "\r\n";
}

std::string errNeedMoreParams(const std::string &serverName, const std::string &nickname, const std::string &command)
{
    return ":" + serverName + " 461 " + nickname + " " + command + " :Not enough parameters\r\n";
}

std::string errInviteOnlyChan(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 473 " + nickname + " " + channelName + " :Cannot join channel (+i)\r\n";
}

std::string errNoSuchChannel(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 403 " + nickname + " " + channelName + " :No such channel\r\n";
}

std::string errNoSuchNick(const std::string &serverName, const std::string &nickname, const std::string &targetNick)
{
    return ":" + serverName + " 401 " + nickname + " " + targetNick + " :No such nick\r\n";
}

std::string errNotOnChannel(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 442 " + nickname + " " + channelName + " :You're not on that channel\r\n";
}

std::string errUserOnChannel(const std::string &serverName, const std::string &nickname, const std::string &targetNick, const std::string &channelName)
{
    return ":" + serverName + " 443 " + nickname + " " + targetNick + " " + channelName + " :is already on channel\r\n";
}

std::string errChanOpPrivsNeeded(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 482 " + nickname + " " + channelName + " :You're not channel operator\r\n";
}

std::string errBadChannelKey(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 475 " + nickname + " " + channelName + " :Cannot join channel (+k)\r\n";
}

std::string errChannelIsFull(const std::string &serverName, const std::string &nickname, const std::string &channelName)
{
    return ":" + serverName + " 471 " + nickname + " " + channelName + " :Cannot join channel (+l)\r\n";
}

std::string errUnknownMode(const std::string &serverName, const std::string &nickname, const std::string &mode)
{
    return ":" + serverName + " 472 " + nickname + " " + mode + " :is unknown mode char to me\r\n";
}

std::string errNicknameInUse(const std::string &serverName, const std::string &nickname, const std::string &targetNick)
{
    return ":" + serverName + " 433 " + nickname + " " + targetNick + " :Nickname is already in use\r\n";
}

std::string errNotRegistered(const std::string &serverName, const std::string &nickname)
{
    return ":" + serverName + " 451 " + nickname + " :You have not registered\r\n";
}

std::string errPasswdMismatch(const std::string &serverName, const std::string &nickname)
{
    return ":" + serverName + " 464 " + nickname + " :Password incorrect\r\n";
}

