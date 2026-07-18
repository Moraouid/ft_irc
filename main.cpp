/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-abbo < sel-abbo@student.1337.ma>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 02:32:37 by sel-abbo          #+#    #+#             */
/*   Updated: 2026/07/16 02:32:57 by sel-abbo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int ac, char *av[])
{
    // if (ac != 3)
    //     return 0;
    Server server;
    server.init_connection();
    server.run_server();
}
