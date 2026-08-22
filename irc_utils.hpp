#ifndef IRC_UTILS_HPP
#define IRC_UTILS_HPP

#include <string>
#include <vector>

std::vector<std::string> splitString(const std::string &input, const std::string &delimiter);
std::string generateTimestamp();
void debugPrint(const std::string &message);
std::string formatReply(const std::string &code, const std::string &nickname, const std::string &message);
std::string formatPrivmsg(const std::string &nickname, const std::string &username, const std::string &target, const std::string &text);
std::string formatNotice(const std::string &target, const std::string &message);

std::string rplWelcome(const std::string &serverName, const std::string &nickname);
std::string rplTopic(const std::string &serverName, const std::string &nickname, const std::string &channel, const std::string &topic);
std::string rplInviting(const std::string &serverName, const std::string &nickname, const std::string &channel, const std::string &targetNick);
std::string inviteMsg(const std::string &nickname, const std::string &username, const std::string &ipAddress, const std::string &targetNick, const std::string &channelName);
std::string rplJoin(const std::string &nickname, const std::string &username, const std::string &ipAddress, const std::string &channelName);
std::string errNeedMoreParams(const std::string &serverName, const std::string &nickname, const std::string &command);
std::string errInviteOnlyChan(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errNoSuchChannel(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errNoSuchNick(const std::string &serverName, const std::string &nickname, const std::string &targetNick);
std::string errNotOnChannel(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errUserOnChannel(const std::string &serverName, const std::string &nickname, const std::string &targetNick, const std::string &channelName);
std::string errChanOpPrivsNeeded(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errBadChannelKey(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errChannelIsFull(const std::string &serverName, const std::string &nickname, const std::string &channelName);
std::string errUnknownMode(const std::string &serverName, const std::string &nickname, const std::string &mode);
std::string errNicknameInUse(const std::string &serverName, const std::string &nickname, const std::string &targetNick);
std::string errNotRegistered(const std::string &serverName, const std::string &nickname);
std::string errPasswdMismatch(const std::string &serverName, const std::string &nickname);

#endif
