/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 02:32:37 by sel-abbo          #+#    #+#             */
/*   Updated: 2026/08/23 01:21:05 by isakrout         ###   ########.fr       */
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

        int port_num = std::atoi(av[1]);
        Server server(port_num, av[2]);

        signal(SIGINT, handle_sigint);
        server.init_connection();
        server.run_server();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}
