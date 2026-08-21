/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 03:33:56 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/21 04:22:49 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>

class Bot
{
    private:
        std::string help_cmd(std::string, int);
        std::string time_cmd(std::string, int);
        std::string users_cmd(std::string, int);
    public:
        std::string bot_handle(std::string msg, std::string nickname, int user_count);
};

#endif
