#include "client.hpp"
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
			if (cmd[spacePos + 1] == ':')
			{
				arg = cmd.substr(spacePos + 2);
				scmd->args.push_back(arg);
				break;
			}
			arg = cmd.substr(spacePos + 1, nextSpacePos - spacePos - 1);
			if (nextSpacePos == std::string::npos)
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
			handlePassCommand(*this, *scmd, password);
		else if (scmd->cmd == "NICK")
			handleNickCommand(*this, *scmd, clients);
		else if (scmd->cmd == "USER")
			handleUserCommand(*this, *scmd);
		else
			appendOut(errNotRegistered("irc", "*"));
		setState();
		if (getState() == REGISTERED)
		{
			appendOut(rplWelcome("irc", nickname));
			std::string bot_msg = ":loffi!bot@localhost PRIVMSG " + nickname + " :Hello! I am loffi, the server bot. send !help for showing commands!\r\n";
			appendOut(bot_msg);
		}
	}
	else
	{
		if (scmd->cmd == "NICK")
			handleNickCommand(*this, *scmd, clients);
		else if (scmd->cmd == "PRIVMSG")
			handlePrivmsgCommand(*this, *scmd, clients, channels);
		else if (scmd->cmd == "JOIN")
			handleJoinCommand(*this, *scmd, clients, channels);
		else if (scmd->cmd == "INVITE")
			handleInvitCommand(*this, *scmd, clients, channels);
		else if (scmd->cmd == "KICK")
			handleKickCommand(*this, *scmd, clients, channels);
		else if (scmd->cmd == "TOPIC")
			handleTopicCommand(*this, *scmd, channels);
		else if (scmd->cmd == "MODE")
			handleModeCommand(*this, *scmd, clients, channels);
		else
			appendOut(errUnknownCommand("irc", nickname, scmd->cmd));
	}
}
