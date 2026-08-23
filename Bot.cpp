/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 03:36:55 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/23 01:13:09 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"



Bot::Bot()
{
    
}

Bot::Bot(const Bot& other)
{
    (void)other;
}

Bot& Bot::operator=(const Bot& other)
{
    (void)other;
    return *this; 
}

Bot::~Bot()
{
    
}




std::string Bot::help_cmd(std::map<int, Client> &clts, std::map<std::string, Channel>& channels)
{
    (void)clts;
    (void)channels;
    return "availble commands: !help, !time, !users, !channels";
}

std::string Bot::time_cmd(std::map<int, Client> &clts, std::map<std::string, Channel>& channels)
{
    (void)clts;
    (void)channels;
    std::time_t cr_time = time(NULL);

    std::string str_time(std::ctime(&cr_time));
    
    return "The current time is: " + str_time;
}

std::string Bot::users_cmd(std::map<int, Client> &clts, std::map<std::string, Channel>& channels)
{
    (void)channels;
    std::stringstream str_stream;

    str_stream << clts.size() - 1;

    std::string u_count = str_stream.str();

    std::map<int, Client>::iterator it = clts.begin();
    std::string users_names;
    it++;
    for (;it != clts.end(); it++)
        users_names.append(it->second.getNickname() + " ");
    return "the users in this server counted to: " + u_count + " the users: " + users_names;
}

std::string Bot::channels_cmd(std::map<int, Client> &clts, std::map<std::string, Channel>& channels)
{
    (void)clts;
    if (channels.size() == 0)
        return "The server has no channels";
    std::string message;
    std::string channel_count;
    std::stringstream channel_count_stream;
    std::string channel_names;

    channel_count_stream << channels.size();
    channel_count = channel_count_stream.str();
    
    message.append("The server has " + channel_count + " channels as follows: =======> ");
   
    std::map<std::string, Channel>::iterator it = channels.begin();
    for (;it != channels.end(); it++)
        message.append(it->first + " ");
    return message;
}


std::string Bot::bot_handle(std::string msg, std::map<int, Client> &clts, std::map<std::string, Channel>& channels)
{
    std::vector<std::string (Bot::*)(std::map<int, Client>&, std::map<std::string, Channel>&)> cmds_fun;
    cmds_fun.push_back(&Bot::help_cmd);
    cmds_fun.push_back(&Bot::time_cmd);
    cmds_fun.push_back(&Bot::users_cmd);
    cmds_fun.push_back(&Bot::channels_cmd);

    std::vector<std::string> cmds_name;
    cmds_name.push_back("!help");
    cmds_name.push_back("!time");
    cmds_name.push_back("!users");
    cmds_name.push_back("!channels");
    
    for (size_t i = 0; i < cmds_name.size(); i++)
    {
        if (msg == cmds_name[i])
            return (this->*cmds_fun[i])(clts, channels);
    }
    
    return "I don't know that command. Type !help.";
}
