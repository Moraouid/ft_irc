#include "command.hpp"

void handlePassCommand(Client &client, const c_cmd &scmd, const std::string &serverPassword)
{
	if (scmd.args.empty() || scmd.args[0].empty() || scmd.args[0] != serverPassword)
	{
		client.appendOut(errPasswdMismatch("irc", "*"));
		return;
	}
	client.setPass(scmd.args[0]);
	client.appendOut(formatReply("001", "*", "Password Accepted"));
}

void handleNickCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients)
{
	if (scmd.args.empty() || scmd.args[0].empty())
	{
		client.appendOut(errNeedMoreParams("irc", "*", "NICK"));
		return;
	}
	std::string oldNick = client.getNickname();
	client.setNickname(scmd.args[0]);
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == client.getNickname() && it->first != client.getFd())
		{
			client.setNickname(client.getNickname() + "_");
			it = clients.begin();
			client.appendOut(errNicknameInUse("irc", "*", client.getNickname()));
		}
	}
	if (oldNick.empty())
		client.appendOut(formatNotice(client.getNickname(), "Nickname set successfully."));
	else
		client.appendOut(setNickname(oldNick, client.getNickname(), client.getUsername()));
}

void handleUserCommand(Client &client, const c_cmd &scmd)
{
	if (scmd.args.empty() || scmd.args[0].empty())
	{
		client.appendOut(errNeedMoreParams("irc", "*", "USER"));
		return;
	}
	client.setUsername(scmd.args[0]);
	client.appendOut(formatNotice(client.getNickname(), "Username set successfully."));
}

void handlePrivmsgCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	if (scmd.args.size() < 2 || scmd.args[0].empty() || scmd.args[1].empty())
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "PRIVMSG"));
		return;
	}
	std::string target = scmd.args[0];
	std::string message = formatPrivmsg(client.getNickname(), client.getUsername(), target, scmd.args[1]);

	if (target[0] == '#')
	{
		std::map<std::string, Channel>::iterator it = channels.find(target);
		if (it != channels.end() && it->second.hasMember(client.getFd()))
		{
			it->second.broadcast(message, client.getFd(), clients);
			return;
		}
		client.appendOut(errNoSuchChannel("irc", client.getNickname(), target));
		return;
	}
	if (target == "loffi")
	{
		Bot bot;
		std::string bot_response = bot.bot_handle(scmd.args[1], clients, channels);
		std::string bot_msg = ":loffi!bot@localhost PRIVMSG " + client.getNickname() + " :" + bot_response + "\r\n";
		client.appendOut(bot_msg);
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
	client.appendOut(errNoSuchNick("irc", client.getNickname(), target));
}

void handleJoinCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	if (scmd.args.empty() || scmd.args[0].empty())
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "JOIN"));
		return;
	}
	std::string channelName = scmd.args[0];
	std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		channels.insert(std::make_pair(channelName, Channel(channelName)));
		chIt = channels.find(channelName);
		chIt->second.addOperator(client.getFd());
	}
	if (chIt->second.getIsLocked())
	{
		if (scmd.args.size() < 2 || scmd.args[1] != chIt->second.getKey())
		{
			client.appendOut(errBadChannelKey("irc", client.getNickname(), channelName));
			return;
		}
	}
	if (chIt->second.getIsPrivate() && !chIt->second.isOperator(client.getFd()) && !chIt->second.isInvited(client.getFd()))
	{
		client.appendOut(errInviteOnlyChan("irc", client.getNickname(), channelName));
		return;
	}
	if (chIt->second.hasUserLimit() && (int)chIt->second.getMembers().size() >= chIt->second.getUserLimit())
	{
		client.appendOut(errChannelIsFull("irc", client.getNickname(), channelName));
		return;
	}
	chIt->second.addMember(client.getFd());
	chIt->second.removeInvitedMember(client.getFd());
	client.joinChannel(channelName);
	std::string joinMessage = rplJoin(client.getNickname(), client.getUsername(), "localhost", channelName);
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
		if (*it == client.getFd())
			continue;
		std::map<int, Client>::iterator clientIt = clients.find(*it);
		if (clientIt != clients.end())
			clientIt->second.appendOut(joinMessage);
	}
	client.appendOut(joinMessage);
	client.appendOut(formatReply("353", client.getNickname(), "= " + channelName + " :" + namesList));
	client.appendOut(formatReply("366", client.getNickname(), channelName + " :End of NAMES list"));
	client.appendOut(rplTopic("irc", client.getNickname(), channelName, chIt->second.getTopic()));
	debugPrint("Client " + client.getNickname() + " joined " + channelName);
}

void handleInvitCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	if (scmd.args.size() < 2)
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "INVITE"));
		return;
	}
	std::string targetNick = scmd.args[0];
	std::string channelName = scmd.args[1];
	std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		client.appendOut(errNoSuchChannel("irc", client.getNickname(), channelName));
		return;
	}
	if (!chIt->second.hasMember(client.getFd()))
	{
		client.appendOut(errNotOnChannel("irc", client.getNickname(), channelName));
		return;
	}
	if (!chIt->second.isOperator(client.getFd()))
	{
		client.appendOut(errChanOpPrivsNeeded("irc", client.getNickname(), channelName));
		return;
	}
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == targetNick)
		{
			if (chIt->second.hasMember(it->first))
			{
				client.appendOut(errUserOnChannel("irc", client.getNickname(), targetNick, channelName));
				return;
			}
			chIt->second.addInvitedMember(it->first);
			client.appendOut(rplInviting("irc", client.getNickname(), channelName, targetNick));
			it->second.appendOut(inviteMsg(client.getNickname(), client.getUsername(), "localhost", targetNick, channelName));
			return;
		}
	}
	client.appendOut(errNoSuchNick("irc", client.getNickname(), targetNick));
}

void handleKickCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	if (scmd.args.size() < 2 || scmd.args[0].empty() || scmd.args[1].empty())
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "KICK"));
		return;
	}
	std::string channelName = scmd.args[0];
	std::string targetNick = scmd.args[1];
	std::string reason = "Kicked";
	if (scmd.args.size() >= 3 && !scmd.args[2].empty())
		reason = scmd.args[2];

	std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		client.appendOut(errNoSuchChannel("irc", client.getNickname(), channelName));
		return;
	}

	if (!chIt->second.hasMember(client.getFd()))
	{
		client.appendOut(errNotOnChannel("irc", client.getNickname(), channelName));
		return;
	}

	if (!chIt->second.isOperator(client.getFd()))
	{
		client.appendOut(errChanOpPrivsNeeded("irc", client.getNickname(), channelName));
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
		client.appendOut(errNoSuchNick("irc", client.getNickname(), targetNick));
		return;
	}

	if (!chIt->second.hasMember(targetFd))
	{
		client.appendOut(errUserOnChannel("irc", client.getNickname(), targetNick, channelName));
		return;
	}

	chIt->second.removeMember(targetFd);
	clients[targetFd].leaveChannel(channelName);
	if (chIt->second.getMembers().empty())
	{
		channels.erase(chIt);
	}

	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
	clients[targetFd].appendOut(kickMsg);
	chIt->second.broadcast(kickMsg, client.getFd(), clients);
	client.appendOut(kickMsg);
	debugPrint("Client " + client.getNickname() + " kicked " + targetNick + " from " + channelName);
}

void handleTopicCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	std::cout << "Handling TOPIC command for client: " << client.getNickname() << std::endl;
	if (scmd.args.size() < 2)
	{
		if (scmd.args.size() == 1)
		{
			std::string channelName = scmd.args[0];
			std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
			std::cout << "Topic for channel " << channelName << ": " << chIt->second.getTopic() << std::endl;
			if (chIt != channels.end())
			{
				if (chIt->second.getTopic().empty())
				{
					client.appendOut(formatReply("331", client.getNickname(), channelName + " :No topic is set"));
				}
				else
					client.appendOut(rplTopic("irc", client.getNickname(), channelName, chIt->second.getTopic()));
				return;
			}
			else
			{
				client.appendOut(errNoSuchChannel("irc", client.getNickname(), channelName));
				return;
			}
		}
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "TOPIC"));
		return;
	}
	std::string channelName = scmd.args[0];
	std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
	if (chIt != channels.end())
	{
		std::string newTopic = scmd.args[1];
		for (size_t i = 2; i < scmd.args.size(); ++i)
			newTopic += " " + scmd.args[i];
		chIt->second.setTopic(newTopic);
		std::string topicMessage = rplTopicChanged(client.getNickname(), client.getUsername(), channelName, newTopic);
		for (std::vector<int>::const_iterator it = chIt->second.getMembers().begin(); it != chIt->second.getMembers().end(); ++it)
		{
			std::map<int, Client>::iterator clientIt = clients.find(*it);
			if (clientIt != clients.end())
				clientIt->second.appendOut(topicMessage);
		}
	}
	else
		client.appendOut(errNoSuchChannel("irc", client.getNickname(), channelName));
}

static void modelCommadd(Client &client, const c_cmd &scmd, std::map<std::string, Channel>::iterator chIt, std::string channelName)
{
	if (scmd.args.size() < 3)
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}
	std::string limitStr = scmd.args[2];
	std::istringstream iss(limitStr);
	int limit = 0;
	if (!(iss >> limit) || limit <= 0)
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}
	chIt->second.setLimit(limit);
	client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "+l " + limitStr));
}

static void modekCommadd(Client &client, const c_cmd &scmd, std::map<std::string, Channel>::iterator chIt, std::string channelName)
{
	if (scmd.args.size() < 3 || scmd.args[2].empty())
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}
	chIt->second.setKey(scmd.args[2]);
	client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "+k"));
}

static void modePlusoCommadd(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel>::iterator chIt, std::string channelName)
{
	if (scmd.args.size() < 3)
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}
	std::string targetNick = scmd.args[2];
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == targetNick)
		{
			if (!chIt->second.hasMember(it->first))
			{
				client.appendOut(formatReply("441", client.getNickname(), targetNick + " " + channelName + " :They aren't on that channel"));
				return;
			}
			chIt->second.addOperator(it->first);
			client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "+o " + targetNick));
			return;
		}
	}
	client.appendOut(errNoSuchNick("irc", client.getNickname(), targetNick));
}

static void modeMinusoCommadd(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel>::iterator chIt, std::string channelName)
{
	if (scmd.args.size() < 3)
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}
	std::string targetNick = scmd.args[2];
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == targetNick)
		{
			if (!chIt->second.hasMember(it->first))
			{
				client.appendOut(errUserNotOnChannel("irc", client.getNickname(), targetNick, channelName));
				return;
			}
			chIt->second.removeOperator(it->first);
			client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "-o " + targetNick));
			return;
		}
	}
	client.appendOut(errNoSuchNick("irc", client.getNickname(), targetNick));
}

void handleModeCommand(Client &client, const c_cmd &scmd, std::map<int, Client> &clients, std::map<std::string, Channel> &channels)
{
	if (scmd.args.empty() || scmd.args[0].empty())
	{
		client.appendOut(errNeedMoreParams("irc", client.getNickname(), "MODE"));
		return;
	}

	std::string channelName = scmd.args[0];
	std::map<std::string, Channel>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		client.appendOut(errNoSuchChannel("irc", client.getNickname(), channelName));
		return;
	}

	if (scmd.args.size() < 2)
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
		client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, modes));
		return;
	}

	if (!chIt->second.isOperator(client.getFd()))
	{
		client.appendOut(errChanOpPrivsNeeded("irc", client.getNickname(), channelName));
		return;
	}

	std::string modeChange = scmd.args[1];
	if (modeChange == "+i")
	{
		chIt->second.setPrivate(true);
		client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "+i"));
	}
	else if (modeChange == "-i")
	{
		chIt->second.setPrivate(false);
		client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "-i"));
	}

	else if (modeChange == "+l")
		modelCommadd(client, scmd, chIt, channelName);
	else if (modeChange == "-l")
	{
		chIt->second.removeLimit();
		client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "-l"));
	}

	else if (modeChange == "+k")
		modekCommadd(client, scmd, chIt, channelName);
	else if (modeChange == "-k")
	{
		chIt->second.removeKey();
		client.appendOut(rplChannelMode("irc", client.getNickname(), channelName, "-k"));
	}

	else if (modeChange == "+o")
		modePlusoCommadd(client, scmd, clients, chIt, channelName);
	else if (modeChange == "-o")
		modeMinusoCommadd(client, scmd, clients, chIt, channelName);
	else
		client.appendOut(errUnknownMode("irc", client.getNickname(), modeChange));
}