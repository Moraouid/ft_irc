/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 03:33:56 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/23 01:11:23 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <map>
#include "client.hpp"
#include "channel.hpp"

class Bot
{
    private:
        std::string help_cmd(std::map<int, Client>& clts, std::map<std::string, Channel>& channels);
        std::string time_cmd(std::map<int, Client>& clts, std::map<std::string, Channel>& channels);
        std::string users_cmd(std::map<int, Client>& clts, std::map<std::string, Channel>& channels);
        std::string channels_cmd(std::map<int, Client> &clts, std::map<std::string, Channel>& channels);
    public:
        Bot();
        Bot(const Bot& other);
        Bot& operator=(const Bot& other);
        ~Bot();
        std::string bot_handle(std::string msg, std::map<int, Client> &clts, std::map<std::string, Channel> &Channels);
};

#endif
