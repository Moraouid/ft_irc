/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 02:32:37 by sel-abbo          #+#    #+#             */
/*   Updated: 2026/08/14 03:20:20 by isakrout         ###   ########.fr       */
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
    (void)av;
    (void)ac;

    Server server;

    signal(SIGINT, handle_sigint);
    server.init_connection();
    server.run_server();
}
