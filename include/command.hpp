#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "client.hpp"
#include "irc_utils.hpp"
#include "Bot.hpp"
#include <map>

typedef struct command c_cmd;

void handlePassCommand(Client &client, const c_cmd &scmd, const std::string &serverPassword);
void handleNickCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients);
void handleUserCommand(Client &client, const c_cmd &scmd);
void handlePrivmsgCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
void handleJoinCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
void handleInvitCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
void handleKickCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
void handleTopicCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);
void handleModeCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels);

#endif