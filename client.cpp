#include "client.hpp"
#include "replies.hpp"
#include "Bot.hpp"

#include <sstream>



Client::Client()
{
	fd = -1;
	ipAddress = "";
	nickname = "";
	username = "";
	pass = "";
	state = UNREGISTERED;
	inBuffer = "";
	outBuffer = "";
}

Client::Client(int fd, std::string ip)
{
	this->fd = fd;
	this->ipAddress = ip;
	nickname = "";
	username = "";
	pass = "";
	state = UNREGISTERED;
	inBuffer = "";
	outBuffer = "";
}

Client::Client(const Client &copy)
{
	fd = copy.fd;
	ipAddress = copy.ipAddress;
	nickname = copy.nickname;
	username = copy.username;
	pass = copy.pass;
	state = copy.state;
	inBuffer = copy.inBuffer;
	outBuffer = copy.outBuffer;
}

Client &Client::operator=(const Client &assign)
{
	if (this != &assign)
	{
		fd = assign.fd;
		ipAddress = assign.ipAddress;
		nickname = assign.nickname;
		username = assign.username;
		pass = assign.pass;
		state = assign.state;
		inBuffer = assign.inBuffer;
		outBuffer = assign.outBuffer;
	}
	return *this;
}

Client::~Client() {}

int Client::getFd() const { return fd; }

std::string Client::getNickname() const { return nickname; }

std::string Client::getUsername() const { return username; }

RegistrationState Client::getState() const { return state; }

std::string &Client::getInBuffer() { return inBuffer; }

std::string &Client::getOutBuffer() { return outBuffer; }

const std::vector<std::string> &Client::getJoinedChannels() const { return joinedChannels; }

bool Client::isInChannel(const std::string &channelName) const
{
	for (std::vector<std::string>::const_iterator it = joinedChannels.begin(); it != joinedChannels.end(); ++it)
	{
		if (*it == channelName)
			return true;
	}
	return false;
}

void Client::setNickname(std::string nick) { nickname = nick; }

void Client::setUsername(std::string user) { username = user; }

void Client::setState()
{
	if (username.empty() || nickname.empty() || pass.empty())
		state = UNREGISTERED;
	else
		state = REGISTERED;
}

void Client::appendIn(char data[1000], int rd) { inBuffer.append(data, rd); }

void Client::appendOut(std::string data) { outBuffer += data; }

void Client::joinChannel(const std::string &channelName)
{
	if (!isInChannel(channelName))
		joinedChannels.push_back(channelName);
}

void Client::leaveChannel(const std::string &channelName)
{
	for (std::vector<std::string>::iterator it = joinedChannels.begin(); it != joinedChannels.end(); ++it)
	{
		if (*it == channelName)
		{
			joinedChannels.erase(it);
			return;
		}
	}
}

void Client::clearInBuffer() { inBuffer.clear(); }

void Client::parseCommand(std::string cmd, c_cmd *scmd)
{
	scmd->cmd = "";
	scmd->args.clear();

	size_t spacePos = cmd.find(' ');
	if (spacePos != std::string::npos)
	{
		scmd->cmd = cmd.substr(0, spacePos);
		while (spacePos != std::string::npos)
		{
			size_t nextSpacePos = cmd.find(' ', spacePos + 1);
			std::string arg;
			if (nextSpacePos != std::string::npos)
			{
				if (cmd[spacePos + 1] == ':')
				{
					arg = cmd.substr(spacePos + 2);
					scmd->args.push_back(arg);
					break;
				}
				arg = cmd.substr(spacePos + 1, nextSpacePos - spacePos - 1);
			}
			else
			{
				arg = cmd.substr(spacePos + 1);
			}
			if (!arg.empty())
				scmd->args.push_back(arg);
			spacePos = nextSpacePos;
		}
	}
	else
	{
		scmd->cmd = cmd;
	}
}

void Client::handleCommand(c_cmd *scmd, std::string password, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	std::ostringstream oss;
	oss << fd;
	debugPrint("Handling command: " + scmd->cmd + " for fd " + oss.str());

	if (getState() == UNREGISTERED)
	{
		if (scmd->cmd == "PASS")
		{
			if (scmd->args.empty() || scmd->args[0].empty() || scmd->args[0] != password)
			{
				appendOut(formatReply("464", "*", "Incorrect password"));
				return;
			}
			setPass(scmd->args[0]);
			appendOut(formatReply("001", "*", "Password accepted"));
		}
		else if (scmd->cmd == "NICK")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut(formatReply("431", "*", "No nickname given"));
				return;
			}
			setNickname(scmd->args[0]);
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (it->second.getNickname() == getNickname() && it->first != getFd())
				{
					setNickname(getNickname() + "_");
					it = clients.begin();
					appendOut(formatReply("433", getNickname(), "Nickname is already in use"));
				}
			}
		}
		else if (scmd->cmd == "USER")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut(formatReply("461", nickname, "USER :Not enough parameters"));
				return;
			}
			setUsername(scmd->args[0]);
			appendOut(formatNotice(nickname, "Username set successfully."));
		}
		else
		{
			appendOut(formatReply("451", "*", "You have not registered"));
		}
		setState();
		if (getState() == REGISTERED)
		{
			appendOut(formatReply("001", nickname, "Welcome to the Internet Relay Network"));
			std::string bot_msg = ":loffi!bot@localhost PRIVMSG " + nickname + " :Hello! I am loffi, the server bot. send !help for showing commands!\r\n";
			appendOut(bot_msg);
		}
	}
	else
	{
		if (scmd->cmd == "PRIVMSG" && scmd->args.size() >= 2)
		{
			std::string target = scmd->args[0];
			std::string message = formatPrivmsg(nickname, username, target, scmd->args[1]);

			if (target[0] == '#')
			{
				std::map<std::string, Channel>::iterator it = channels.find(target);
				if (it != channels.end() && it->second.hasMember(fd))
				{
					it->second.broadcast(message, fd, clients);
					return;
				}
				appendOut(formatReply("403", nickname, target + " :No such channel"));
				return;
			}

			if (target == "loffi")
			{
				Bot bot;
				std::string bot_response = bot.bot_handle(scmd->args[1], nickname, clients.size() -1);
				std::string bot_msg = ":loffi!bot@localhost PRIVMSG " + nickname + " :" + bot_response + "\r\n";
				appendOut(bot_msg);
				return ;
			}
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (it->second.getNickname() == target)
				{
					it->second.appendOut(message);
					return;
				}
			}
			appendOut(formatReply("401", nickname, target + " :No such nick"));
		}
		else if (scmd->cmd == "JOIN")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut(formatReply("461", nickname, "JOIN :Not enough parameters"));
				return;
			}
			std::string channelName = scmd->args[0];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt == channels.end())
			{
				channels.insert(std::make_pair(channelName, Channel(channelName)));
				chIt = channels.find(channelName);
				chIt->second.addOperator(fd);
			}
			if (chIt->second.getIsLocked())
			{
				if (scmd->args.size() < 2 || scmd->args[1] != chIt->second.getKey())
				{
					appendOut(formatReply(ERR_BADCHANNELKEY, nickname, channelName + " :Cannot join channel (+k)"));
					return;
				}
			}
			if (chIt->second.getIsPrivate() && !chIt->second.isOperator(fd) && !chIt->second.isInvited(fd))
			{
				appendOut(errInviteOnlyChan("irc", nickname, channelName));
				return;
			}
			if (chIt->second.hasUserLimit() && (int)chIt->second.getMembers().size() >= chIt->second.getUserLimit())
			{
				appendOut(errChannelIsFull("irc", nickname, channelName));
				return;
			}
			chIt->second.addMember(fd);
			chIt->second.removeInvitedMember(fd);
			joinChannel(channelName);
			std::string joinMessage = rplJoin(nickname, username, "localhost", channelName);
			std::string namesList;
			for (std::vector<int>::const_iterator it = chIt->second.getMembers().begin(); it != chIt->second.getMembers().end(); ++it)
			{
				std::map<int, Client>::iterator clientIt = clients.find(*it);
				if (clientIt == clients.end())
					continue;
				if (!namesList.empty())
					namesList += " ";
				if (chIt->second.isOperator(*it))
					namesList += "@";
				namesList += clientIt->second.getNickname();
			}
			for (std::vector<int>::const_iterator it = chIt->second.getMembers().begin(); it != chIt->second.getMembers().end(); ++it)
			{
				if (*it == fd)
					continue;
				std::map<int, Client>::iterator clientIt = clients.find(*it);
				if (clientIt != clients.end())
				clientIt->second.appendOut(joinMessage);
			}
			appendOut(joinMessage);
			appendOut(formatReply("353", nickname, "= " + channelName + " :" + namesList));
			appendOut(formatReply("366", nickname, channelName + " :End of NAMES list"));
			debugPrint("Client " + nickname + " joined " + channelName);
		}
		else if (scmd->cmd == "INVITE")
		{
			if (scmd->args.size() < 2)
			{
				appendOut(formatReply("461", nickname, "INVITE :Not enough parameters"));
				return;
			}
			std::string targetNick = scmd->args[0];
			std::string channelName = scmd->args[1];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt == channels.end())
			{
				appendOut(formatReply("403", nickname, channelName + " :No such channel"));
				return;
			}
			if (!chIt->second.hasMember(fd))
			{
				appendOut(formatReply("442", nickname, channelName + " :You're not on that channel"));
				return;
			}
			if (!chIt->second.isOperator(fd))
			{
				appendOut(formatReply("482", nickname, channelName + " :You're not channel operator"));
				return;
			}
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (it->second.getNickname() == targetNick)
				{
					if (chIt->second.hasMember(it->first))
					{
						appendOut(formatReply("443", nickname, targetNick + " " + channelName + " :is already on channel"));
						return;
					}
					chIt->second.addInvitedMember(it->first);
					appendOut(rplInviting("irc", nickname, channelName, targetNick));
					it->second.appendOut(inviteMsg(nickname, username, "localhost", targetNick, channelName));
					return;
				}
			}
			appendOut(formatReply("401", nickname, targetNick + " :No such nick"));
		}
			else if (scmd->cmd == "KICK")
			{
				if (scmd->args.size() < 2 || scmd->args[0].empty() || scmd->args[1].empty())
				{
					appendOut(errNeedMoreParams("irc", nickname, "KICK"));
					return;
				}
				std::string channelName = scmd->args[0];
				std::string targetNick = scmd->args[1];
				std::string reason = "Kicked";
				if (scmd->args.size() >= 3 && !scmd->args[2].empty())
					reason = scmd->args[2];
				std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
				if (chIt == channels.end())
				{
					appendOut(errNoSuchChannel("irc", nickname, channelName));
					return;
				}
				if (!chIt->second.hasMember(fd))
				{
					appendOut(errNotOnChannel("irc", nickname, channelName));
					return;
				}
				if (!chIt->second.isOperator(getFd()))
				{
					appendOut(errChanOpPrivsNeeded("irc", nickname, channelName));
					return;
				}
				int targetFd = -1;
				for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					if (it->second.getNickname() == targetNick)
					{
						targetFd = it->first;
						break;
					}
				}
				if (targetFd == -1)
				{
					appendOut(errNoSuchNick("irc", nickname, targetNick));
					return;
				}
				if (!chIt->second.hasMember(targetFd))
				{
					appendOut(errUserOnChannel("irc", nickname, targetNick, channelName));
					return;
				}
				// remove target from channel and notify
				chIt->second.removeMember(targetFd);
				clients[targetFd].leaveChannel(channelName);
				std::string kickMsg = ":" + nickname + "!" + username + "@localhost KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
				// send to kicked user
				clients[targetFd].appendOut(kickMsg);
				// broadcast to remaining members
				chIt->second.broadcast(kickMsg, getFd(), clients);
				// send to kicker as confirmation
				appendOut(kickMsg);
				debugPrint("Client " + nickname + " kicked " + targetNick + " from " + channelName);
			}
		else if (scmd->cmd == "TOPIC")
		{
			if (scmd->args.size() < 2)
			{
				if (scmd->args.size() == 1)
				{
					std::string channelName = scmd->args[0];
					std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
					if (chIt != channels.end())
					{
						appendOut(formatReply("332", nickname, channelName + " :" + chIt->second.getTopic()));
						return;
					}
					else
					{
						appendOut(formatReply("403", nickname, channelName + " :No such channel"));
						return;
					}
				}
				appendOut(formatReply("461", nickname, "TOPIC :Not enough parameters"));
				return;
			}
			std::string channelName = scmd->args[0];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt != channels.end())
			{
				chIt->second.setTopic(scmd->args[1]);
				appendOut(formatReply("332", nickname, channelName + " :" + scmd->args[1]));
			}
			else
				appendOut(formatReply("403", nickname, channelName + " :No such channel"));
		}
		else if (scmd->cmd == "MODE")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
				return;
			}
			std::string channelName = scmd->args[0];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt == channels.end())
			{
				appendOut(formatReply("403", nickname, channelName + " :No such channel"));
				return;
			}
			if (scmd->args.size() < 2)
			{
				std::string modes = "";
				if (chIt->second.getIsPrivate())
					modes += "+i";
				if (chIt->second.hasUserLimit())
				{
					if (!modes.empty())
						modes += " ";
					std::ostringstream ms;
					ms << "+l " << chIt->second.getUserLimit();
					modes += ms.str();
				}
				if (chIt->second.getIsLocked())
				{
					if (!modes.empty())
						modes += " ";
					modes += "+k";
				}
				appendOut(formatReply("324", nickname, channelName + " " + modes));
				return;
			}

			if (!chIt->second.isOperator(getFd()))
			{
				appendOut(formatReply("482", nickname, channelName + " :You're not channel operator"));
				return;
			}

			std::string modeChange = scmd->args[1];
			if (modeChange == "+i")
			{
				chIt->second.setPrivate(true);
				appendOut(formatReply("324", nickname, channelName + " +i"));
			}
			else if (modeChange == "-i")
			{
				chIt->second.setPrivate(false);
				appendOut(formatReply("324", nickname, channelName + " -i"));
			}
			else if (modeChange == "+l")
			{
				if (scmd->args.size() < 3)
				{
					appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
					return;
				}
				std::string limitStr = scmd->args[2];
				std::istringstream iss(limitStr);
				int limit = 0;
				if (!(iss >> limit) || limit <= 0)
				{
					appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
					return;
				}
				chIt->second.setLimit(limit);
				appendOut(formatReply("324", nickname, channelName + " +l " + limitStr));
			}
			else if (modeChange == "-l")
			{
				chIt->second.removeLimit();
				appendOut(formatReply("324", nickname, channelName + " -l"));
			}
			else if (modeChange == "+k")
			{
				if (scmd->args.size() < 3 || scmd->args[2].empty())
				{
					appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
					return;
				}
				chIt->second.setKey(scmd->args[2]);
				appendOut(formatReply("324", nickname, channelName + " +k"));
			}
			else if (modeChange == "-k")
			{
				chIt->second.removeKey();
				appendOut(formatReply("324", nickname, channelName + " -k"));
			}
			else if (modeChange == "+o")
			{
				if (scmd->args.size() < 3)
				{
					appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
					return;
				}
				std::string targetNick = scmd->args[2];
				for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					if (it->second.getNickname() == targetNick)
					{
						if (!chIt->second.hasMember(it->first))
						{
							appendOut(formatReply("441", nickname, targetNick + " " + channelName + " :They aren't on that channel"));
							return;
						}
						chIt->second.addOperator(it->first);
						appendOut(formatReply("324", nickname, channelName + " +o " + targetNick));
						return;
					}
				}
				appendOut(formatReply("401", nickname, targetNick + " :No such nick"));
			}
			else if (modeChange == "-o")
			{
				if (scmd->args.size() < 3)
				{
					appendOut(formatReply("461", nickname, "MODE :Not enough parameters"));
					return;
				}
				std::string targetNick = scmd->args[2];
				for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					if (it->second.getNickname() == targetNick)
					{
						if (!chIt->second.hasMember(it->first))
						{
							appendOut(formatReply("441", nickname, targetNick + " " + channelName + " :They aren't on that channel"));
							return;
						}
						chIt->second.removeOperator(it->first);
						appendOut(formatReply("324", nickname, channelName + " -o " + targetNick));
						return;
					}
				}
				appendOut(formatReply("401", nickname, targetNick + " :No such nick"));
			}
			else
			{
				appendOut(formatReply("472", nickname, modeChange + " :is unknown mode char to me"));
			}
		}
	}
}
