/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 06:52:36 by isakrout          #+#    #+#             */
/*   Updated: 2026/06/29 07:00:04 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>


class Client
{
    private:
        int fd;
        std::string name;
        std::string in_buff;
        std::string out_buff;
    public:
        Client(int fd, std::string name, std::string in, std::string out)
        {
            this->fd = fd;
            this->fd = fd;
            this->in_buff = in;
            this->out_buff = out;
        }
};

class Server
{
    private:
        Client client;
    public:
        void init_connection();
        void handle_connection();
};

#endif
