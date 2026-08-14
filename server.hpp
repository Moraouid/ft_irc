/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 06:52:36 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/14 01:58:12 by isakrout         ###   ########.fr       */
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
#include <map>
#include <exception>
#include "client.hpp"

//class Client
//{
//    public:
//        int fd;
//        std::string name;
//        std::string in_buff;
//        std::string out_buff;
//    public:
//        Client(){}
//        Client(int fd, std::string name, std::string in, std::string out)
//        {
//            this->fd = fd;
//            this->fd = fd;
//            this->in_buff = in;
//            this->out_buff = out;
//        }
//};

class Server
{
    private:
        int port_number;
        std::string password;
        std::map<int, Client> clients;
        std::vector<struct pollfd> poll_fds;
        int listening_fd;
        std::string buffer;
    public:
        class server_errors: public std::exception
        {
            private:
                std::string msg;
            public:
                server_errors();
                server_errors(std::string msg);
                const char *what() throw();
                ~server_errors() throw();
        };
        Server();
        Server(int port_num, std::string pass);
        void init_connection();
        void run_server();
        void handle_connection();
        int handle_arriving_data(size_t *i);    
        int handle_sending_data(size_t *i);
        void close_connection(size_t *i);
        void clear_server();
};

#endif

