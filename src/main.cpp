/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sfaouzi <sfaouzi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 02:32:37 by sel-abbo          #+#    #+#             */
/*   Updated: 2026/08/25 00:06:11 by sfaouzi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int is_run = 1;

void handle_sigint(int)
{
    is_run = 0;
}

int main(int ac, char *av[])
{
    try
    {
        if (ac != 3)
            throw Server::server_errors("provide arguments as follows: ./ircserv <port> <password>");

        std::string port_str = av[1];
        if (port_str.size() > 5)
            throw Server::server_errors("invalid port number. Port number must be between 1 and 65535.");
        int port_num = std::atoi(port_str.c_str());
        if (port_num <= 0 || port_num > 65535)
            throw Server::server_errors("invalid port number. Port number must be between 1 and 65535.");

        Server server(port_num, av[2]);

        signal(SIGINT, handle_sigint);
        server.init_connection();
        server.run_server();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
