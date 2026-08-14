#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>


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

	// Setters
	void setNickname(std::string nick);
	void setUsername(std::string user);
	void setPass(std::string password){
		pass = password;
	};
	void setState();

	void parseCommand(std::string cmd, c_cmd *scmd);
	void handleCommand(c_cmd *scmd, std::string password, std::map<int, Client> &clients);
	void appendIn(char data[1000], int rd);
	void appendOut(std::string data);
	void clearInBuffer();
};

#endif