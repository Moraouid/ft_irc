/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 03:36:55 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/21 04:33:27 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"


std::string Bot::help_cmd(std::string nickname, int user_count)
{
    return "availble commands: !help, !time, !users";
}

std::string Bot::time_cmd(std::string nickname, int user_count)
{
    std::time_t cr_time = time(NULL);

    std::string str_time(std::ctime(&cr_time));
    
    return "The current time is: " + str_time;
}

std::string Bot::users_cmd(std::string nickname, int user_count)
{
    std::stringstream str_stream;

    str_stream << user_count;

    std::string u_count = str_stream.str();

    return "the users in this server counted to: " + u_count;
}

std::string Bot::bot_handle(std::string msg, std::string nickname, int user_count)
{
    std::cout << "----> " << msg << std::endl;
    std::vector<std::string (Bot::*)(std::string, int)> cmds_fun;
    cmds_fun.push_back(&Bot::help_cmd);
    cmds_fun.push_back(&Bot::time_cmd);
    cmds_fun.push_back(&Bot::users_cmd);
    std::vector<std::string> cmds_name;
    cmds_name.push_back(":!help");
    cmds_name.push_back(":!time");
    cmds_name.push_back(":!users");
    
    for (size_t i = 0; i < cmds_name.size(); i++)
    {
        if (msg == cmds_name[i])
            return (this->*cmds_fun[i])(nickname, user_count);
    }
    
    return "I don't know that command. Type !help.";
}
