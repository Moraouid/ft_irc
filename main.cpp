/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 02:32:37 by sel-abbo          #+#    #+#             */
/*   Updated: 2026/07/18 02:14:42 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int ac, char *av[])
{
    (void)av;
    (void)ac;
    // if (ac != 3)
    //     return 0;
    Server server;
    server.init_connection();
    server.run_server();
}
