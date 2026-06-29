/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 06:50:39 by isakrout          #+#    #+#             */
/*   Updated: 2026/06/29 06:59:53 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "server.hpp"





void Server::init_connection()
{
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(55555);
    serverAddress.sin_addr.s_addr = INADDR_ANY;


    int opt = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        std::cout << "setsockopt failed" << std::endl;
    int b = bind(fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (b < 0)
        std::cout << "bind failed" << std::endl;
    int l = listen(fd, 1);
    char buff[100];
    int client_fd = accept(fd, NULL, NULL);
    int rd;
    while ((rd = recv(client_fd, buff, sizeof(buff) - 1, 0)) > 0)
    {
        buff[rd] = '\0';
        handle_connection(buff);
    }
}


int main()
{
    Server server;
    server.init_connection();
}
