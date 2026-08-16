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
				appendOut("Incorrect password. Please try again.\n");
				return;
			}
			setPass(scmd->args[0]);
			appendOut((std::string(":") + "irc" + " 001 " + this->getNickname() + " :Welcome to the Internet Relay Network, " + this->getNickname() + "/r/n"));
		}
		else if (scmd->cmd == "NICK")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut("Nickname cannot be empty.\n");
				return;
			}
			setNickname(scmd->args[0]);
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (it->second.getNickname() == getNickname() && it->first != getFd())
				{
					setNickname(getNickname() + "_");
					it = clients.begin();
					appendOut("Nickname already taken. Changed to: " + getNickname() + "\n");
				}
			}
			appendOut("Nickname set successfully.\n");
		}
		else if (scmd->cmd == "USER")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut("Username cannot be empty.\n");
				return;
			}
			setUsername(scmd->args[0]);
			appendOut("Username set successfully.\n");
		}
		else
		{
			appendOut("You must register first using PASS, NICK, and USER commands.\n");
		}
		setState();
		if (getState() == REGISTERED)
			appendOut("you are a registred\n");
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
				appendOut("Channel not found or you are not in it.\n");
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
			appendOut("User not found.\n");
		}
		else if (scmd->cmd == "JOIN")
		{
			if (scmd->args.empty() || scmd->args[0].empty())
			{
				appendOut("JOIN requires a channel name.\n");
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
			chIt->second.addMember(fd);
			joinChannel(channelName);
			appendOut("Joined channel: " + channelName + "\n");
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
						appendOut("Current topic for " + channelName + ": " + chIt->second.getTopic() + "\n");
						return;
					}
					else
					{
						appendOut("Channel not found: " + channelName + "\n");
						return;
					}
				}
				appendOut("TOPIC requires a channel and a topic.\n");
				return;
			}
			std::string channelName = scmd->args[0];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			if (chIt != channels.end())
			{
				chIt->second.setTopic(scmd->args[1]);
				appendOut("Topic updated for " + channelName + ": " + scmd->args[1] + "\n");
			}
			else
				appendOut("Channel not found: " + channelName + "\n");
		}
		else
		{
			appendOut("Unknown command: " + scmd->cmd + "\n");
		}
	}
}
