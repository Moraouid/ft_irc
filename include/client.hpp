#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include "channel.hpp"
#include "irc_utils.hpp"
#include "command.hpp"

typedef struct command
{
	std::string cmd;
	std::vector<std::string> args;
} c_cmd;

enum RegistrationState
{
	UNREGISTERED,
	REGISTERED
};

class Client
{
private:
	int fd;
	std::string ipAddress;

	std::string nickname;
	std::string username;
	std::string pass;

	RegistrationState state;
	std::string inBuffer;
	std::string outBuffer;
	std::vector<std::string> joinedChannels;

public:
	Client();
	Client(int fd, std::string ip);
	Client(const Client &copy);
	Client &operator=(const Client &assign);
	~Client();

	// Getters
	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	RegistrationState getState() const;
	std::string &getInBuffer();
	std::string &getOutBuffer();
	const std::vector<std::string> &getJoinedChannels() const;
	bool isInChannel(const std::string &channelName) const;

	// Setters
	void setNickname(std::string nick);
	void setUsername(std::string user);
	void setPass(std::string password);
	void setState();

	void joinChannel(const std::string &channelName);
	void leaveChannel(const std::string &channelName);
	void parseCommand(std::string cmd, c_cmd *scmd);
	void handleCommand(c_cmd *scmd, std::string password, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
	void appendIn(char data[1000], int rd);
	void appendOut(std::string data);
	void clearInBuffer();
};

#endif