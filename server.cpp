/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 06:50:39 by isakrout          #+#    #+#             */
/*   Updated: 2026/07/17 23:32:10 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::Server()
{

}

Server::Server(int port_num, std::string pass): port_number(port_num), password(pass)
{

}

Server::server_errors::server_errors()
{
    msg = "server error";
}

Server::server_errors::server_errors(std::string msg)
{
    this->msg = msg;
}


const char* Server::server_errors::what() throw()
{
    return msg.c_str();
}

void Server::handle_connection()
{
    int connect_fd = accept(listening_fd, NULL, NULL);
    poll_fds.push_back({connect_fd, POLLIN, 0});
    
    std::pair<int, Client> cl = std::make_pair(connect_fd, Client(connect_fd, "anonymous", "", ""));
    clients.insert(cl);
    // fcntl(connect_fd, F_SETFL, O_NONBLOCK);
    std::cout << "Client connected on fd " << connect_fd << std::endl;
}

int Server::handle_arriving_data(size_t *i)
{
    char temp_buf[1000];
    int rd = recv(poll_fds[*i].fd, temp_buf, sizeof(temp_buf), 0);
    if (rd > 0)
    {
        clients[poll_fds[*i].fd].in_buff.append(temp_buf, rd);
        std::cout << "From Client " << poll_fds[*i].fd << ": " << clients[poll_fds[*i].fd].in_buff;
        size_t delim;
        std::string cmd;
        while ((delim = clients[poll_fds[*i].fd].in_buff.find("\n")) != std::string::npos)
        {
            cmd = clients[poll_fds[*i].fd].in_buff.substr(0, delim);
            clients[poll_fds[*i].fd].in_buff.erase(0, delim+1);

            std::cout << cmd << std::endl;
        }
        // clients[poll_fds[*i].fd].out_buff = ":myserver 001 wiiik :Welcome HOOOO";
    }
    else if (rd <= 0)
    {
        close_connection(i);
        return 0;
    }
    return 1;
}

int Server::handle_sending_data(size_t *i)
{
    int s_ret;
    std::string buff = clients[poll_fds[*i].fd].out_buff.c_str();
    s_ret = send(poll_fds[*i].fd,  buff.c_str(), buff.size(), 0);
    if (s_ret > 0)
    {
        clients[poll_fds[*i].fd].out_buff.erase(0, s_ret);
    }
    else
    {
        close_connection(i);
        return 0;
    }
    return 1;
}

void Server::close_connection(size_t *i)
{
    int fd = poll_fds[*i].fd;

    close(fd);

    clients.erase(fd);

    poll_fds.erase(poll_fds.begin() + *i);

    std::cout << "client " << fd << " disconnected" << std::endl;
    i--;
}

void Server::run_server()
{
    int poll_ret = 0;
    while (1)
    {
        for (size_t i = 0; i < poll_fds.size(); i++)
        {
            if (poll_fds[i].fd == listening_fd)
                continue;
            if (!clients[poll_fds[i].fd].out_buff.empty())
                poll_fds[i].events |= POLLOUT;
            else
                poll_fds[i].events = POLLIN;
        }
        poll_ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_ret > 0)
        {
            for (size_t i = 0; i < poll_fds.size(); i++)
            {
                if (poll_fds[i].revents & POLLIN)
                {
                    if (poll_fds[i].fd == listening_fd)
                        handle_connection();
                    else if (!handle_arriving_data(&i))
                        continue;
                }
                if (poll_fds[i].revents & POLLOUT)
                {
                        if (!handle_sending_data(&i))
                            continue;
                }
                if (poll_fds[i].revents & (POLLHUP | POLLERR))
                    close_connection(&i);
            }
        }
    }
}

void Server::init_connection()
{
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6667);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    listening_fd  = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_fd < 0)
        throw Server::server_errors("cannot create socket");
    if (setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw Server::server_errors("cannot set socket option");
    if (bind(listening_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        throw Server::server_errors("cannot bind the socket");
    if (listen(listening_fd, 1) < 0)
        throw Server::server_errors("listen failed");
    poll_fds.push_back({listening_fd, POLLIN, 0});
    // fcntl(listening_fd, F_SETFL, O_NONBLOCK);
}

int main(int ac, char *av[])
{
    // if (ac != 3)
    //     return 0;
    Server server;
    server.init_connection();
    server.run_server();
}
