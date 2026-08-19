#include "client.hpp"

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
			appendOut("Username set successfully.\n");
		}
		else
		{
			appendOut(formatReply("451", "*", "You have not registered"));
		}
		setState();
		if (getState() == REGISTERED)
			appendOut(formatReply("001", nickname, "Welcome to the Internet Relay Network"));
	}
	else
	{
		if (scmd->cmd == "PRIVMSG" && scmd->args.size() >= 2)
		{
			std::string target = scmd->args[0];
			std::string message = "Message from " + getNickname() + ": " + scmd->args[1] + "\n";

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
			if (chIt->second.getIsPrivate() && !chIt->second.isOperator(fd) && !chIt->second.isInvited(fd))
			{
				appendOut(formatReply("473", nickname, channelName + " :Cannot join channel (+i)"));
				return;
			}
			if (chIt->second.hasUserLimit() && (int)chIt->second.getMembers().size() >= chIt->second.getUserLimit())
			{
				appendOut(formatReply("471", nickname, channelName + " :Cannot join channel (+l)"));
				return;
			}
			chIt->second.addMember(fd);
			joinChannel(channelName);
			appendOut(formatReply("353", nickname, "= " + channelName + " :@" + nickname));
			appendOut(formatReply("366", nickname, channelName + " :End of NAMES list"));
			debugPrint("Client " + nickname + " joined " + channelName);
		}
		else if (scmd->cmd == "TOPIC")
		{
			if (scmd->args.size() < 2)
			{
				if(scmd->args.size() == 1)
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
		else if (scmd->cmd == "KICK")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut(formatReply("461", nickname, "KICK :Not enough parameters"));
				return;
			}
			std::string channelName = scmd->args[0];
			std::string memberName = scmd->args[1];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt == channels.end())
			{
				appendOut(formatReply("403", nickname, channelName + " :No such channel"));
				return;
			}
			chIt->second.removeMember(fd);
			leaveChannel(channelName);
			appendOut(formatReply("KICK", nickname, channelName + " " + memberName + " :You have been kicked from the channel"));
			debugPrint("Client " + nickname + " kicked " + memberName + " from " + channelName);
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
