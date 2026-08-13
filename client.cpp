#include "client.hpp"

Client::Client()
{
	fd = -1;
	ipAddress = "";
	nickname = "";
	username = "";
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

void Client::clearInBuffer() { inBuffer.clear(); }

void Client::parseCommand(std::string cmd, c_cmd *scmd)
{
	size_t spacePos = cmd.find(' ');
	if (spacePos != std::string::npos)
	{
		scmd->cmd = cmd.substr(0, spacePos);
		while (spacePos != std::string::npos)
		{
			size_t nextSpacePos = cmd.find(' ', spacePos + 1);
			if (nextSpacePos != std::string::npos)
			{
				if (cmd[spacePos + 1] == ':')
				{
					scmd->args.push_back(cmd.substr(spacePos + 2));
					break;
				}
				else
					scmd->args.push_back(cmd.substr(spacePos + 1, nextSpacePos - spacePos - 1));
			}
			else
			{
				scmd->args.push_back(cmd.substr(spacePos + 1));
			}
			spacePos = nextSpacePos;
		}
	}
	else
	{
		scmd->cmd = cmd;
		scmd->args.push_back("");
	}
}

void Client::handleCommand(c_cmd *scmd, std::string password, std::map<int, Client> &clients)
{
	if (getState() == UNREGISTERED)
	{
		if (scmd->cmd == "PASS")
		{
			if(scmd->args[0] != password)
			{
				appendOut("Incorrect password. Please try again.\n");
				return;
			}
			setPass(scmd->args[0]);
			appendOut("Password set successfully.\n");
		}
		else if (scmd->cmd == "NICK")
		{
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
			setUsername(scmd->args[0]);
			appendOut("Username set successfully.\n");
		}
		else
		{
			appendOut("You must register first using PASS, NICK, and USER commands.\n");
		}
		setState();
		if (getState() == REGISTERED)
			appendOut("You are now registered!\n");
	}
	else
	{
		if (scmd->cmd == "PRIVMSG" && scmd->args.size() == 2)
		{
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (it->second.getNickname() == scmd->args[0])
				{
					std::string message = ":myserver PRIVMSG " + scmd->args[0] + " :" + scmd->args[1] + "\n";
					send(it->first, message.c_str(), message.size(), 0);
					it->second.appendOut("Message from " + getNickname() + ": " + scmd->args[1] + "\n");
					return;
				}
			}
			appendOut("User not found.	\n");
		}
		else if (scmd->cmd == "JOIN")
		{
			appendOut("Joined channel: " + scmd->args[0] + "\n");
		}
		else if (scmd->cmd == "PART")
		{
			appendOut("Left channel: " + scmd->args[0] + "\n");
		}
		else
		{
			appendOut("Unknown command: " + scmd->cmd + "\n");
		}
	}
}
