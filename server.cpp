/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: isakrout <isakrout@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 06:50:39 by isakrout          #+#    #+#             */
/*   Updated: 2026/08/21 03:09:27 by isakrout         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

extern int is_run;

Server::Server()
{
}

Server::Server(int port_num, std::string pass) : port_number(port_num), password(pass)
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

const char *Server::server_errors::what() throw()
{
    return msg.c_str();
}

Server::server_errors::~server_errors() throw()
{
}

void Server::handle_connection()
{
    int connect_fd = accept(listening_fd, NULL, NULL);
    if (connect_fd == -1)
        return;

    struct pollfd new_clt;

    new_clt.fd = connect_fd;
    new_clt.events = POLLIN;
    new_clt.revents = 0;
    poll_fds.push_back(new_clt);

    std::pair<int, Client> cl = std::make_pair(connect_fd, Client(connect_fd, ""));
    clients.insert(cl);
    fcntl(connect_fd, F_SETFL, O_NONBLOCK);
    std::cout << "Client connected on fd " << connect_fd << std::endl;
}

int Server::handle_arriving_data(size_t *i)
{
    char temp_buf[1000];
    int rd = recv(poll_fds[*i].fd, temp_buf, sizeof(temp_buf), 0);
    if (rd > 0)
    {
        clients[poll_fds[*i].fd].appendIn(temp_buf, rd);
        size_t delim;
        std::string cmd;
        while ((delim = clients[poll_fds[*i].fd].getInBuffer().find("\n")) != std::string::npos)
        {
            c_cmd scmd;
            scmd.cmd.clear();
            scmd.args.clear();
            if (delim + 1 > 512)
                cmd = clients[poll_fds[*i].fd].getInBuffer().substr(0, 510);
            else
                cmd = clients[poll_fds[*i].fd].getInBuffer().substr(0, delim);
            clients[poll_fds[*i].fd].getInBuffer().erase(0, delim + 1);
            if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
                cmd.erase(cmd.size() - 1);
            if (!cmd.empty())
            {
                clients[poll_fds[*i].fd].parseCommand(cmd, &scmd);
                clients[poll_fds[*i].fd].handleCommand(&scmd, password, clients, channels);
                // parsing and executing logic
            }
        }
        return 1;
    }
    else
        close_connection(i);
    return 0;
}

int Server::handle_sending_data(size_t *i)
{
    int s_ret;
    std::string buff = clients[poll_fds[*i].fd].getOutBuffer().c_str();
    s_ret = send(poll_fds[*i].fd, buff.c_str(), buff.size(), 0);
    if (s_ret > 0)
        clients[poll_fds[*i].fd].getOutBuffer().erase(0, s_ret);
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
    (*i)--;
}

void Server::run_server()
{
    int poll_ret = 0;
    while (is_run)
    {
        for (size_t i = 0; i < poll_fds.size(); i++)
        {
            if (poll_fds[i].fd == listening_fd)
                continue;
            if (!clients[poll_fds[i].fd].getOutBuffer().empty())
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
    clear_server();
}

void Server::init_connection()
{
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6667);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    password = "pass";

    int opt = 1;
    listening_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_fd < 0)
        throw Server::server_errors("cannot create socket");
    if (setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw Server::server_errors("cannot set socket option");
    if (bind(listening_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
        throw Server::server_errors("cannot bind the socket");
    if (listen(listening_fd, 1) < 0)
        throw Server::server_errors("listen failed");

    struct pollfd listen_poll_fd;
    listen_poll_fd.fd = listening_fd;
    listen_poll_fd.events = POLLIN;
    listen_poll_fd.revents = 0;
    poll_fds.push_back(listen_poll_fd);
    fcntl(listening_fd, F_SETFL, O_NONBLOCK);

    Client bot_clt(-1, "");
    bot_clt.setNickname("loffi");
    bot_clt.setUsername("bot");
    bot_clt.setPass(password);
    bot_clt.setState();

    clients[-1] = bot_clt;
}

Channel *Server::getChannel(const std::string &channelName)
{
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
        return NULL;
    return &it->second;
}

bool Server::createChannel(const std::string &channelName)
{
    if (channels.find(channelName) != channels.end())
        return false;
    channels.insert(std::make_pair(channelName, Channel(channelName)));
    return true;
}

void Server::joinChannel(int clientFd, const std::string &channelName)
{
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
        return;
    it->second.addMember(clientFd);
    std::map<int, Client>::iterator clientIt = clients.find(clientFd);
    if (clientIt != clients.end())
        clientIt->second.joinChannel(channelName);
}

void Server::partChannel(int clientFd, const std::string &channelName)
{
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
        return;
    it->second.removeMember(clientFd);
    std::map<int, Client>::iterator clientIt = clients.find(clientFd);
    if (clientIt != clients.end())
        clientIt->second.leaveChannel(channelName);
}

void Server::clear_server()
{
    std::cout << "shutting down the server" << std::endl;

    std::map<int, Client>::iterator it;
    std::string error_message = "ERROR :Server shutting down\r\n";
    for (it = clients.begin(); it != clients.end(); it++)
        send(it->second.getFd(), error_message.c_str(), error_message.size(), 0);

    for (size_t i = 0; i < poll_fds.size(); i++)
        close(poll_fds[i].fd);
}
