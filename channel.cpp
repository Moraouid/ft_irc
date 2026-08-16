#include "channel.hpp"
#include "client.hpp"

Channel::Channel() : name(""), topic("default topic") {}

Channel::Channel(const std::string &channelName) : name(channelName), topic("default topic") {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
    return name;
}

const std::string &Channel::getTopic() const
{
    return topic;
}

void Channel::setTopic(const std::string &newTopic)
{
    topic = newTopic;
}

bool Channel::hasMember(int fd) const
{
    for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it == fd)
            return true;
    }
    return false;
}

bool Channel::isOperator(int fd) const
{
    for (std::vector<int>::const_iterator it = operators.begin(); it != operators.end(); ++it)
    {
        if (*it == fd)
            return true;
    }
    return false;
}

void Channel::addMember(int fd)
{
    if (!hasMember(fd))
        members.push_back(fd);
}

void Channel::removeMember(int fd)
{
    for (std::vector<int>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it == fd)
        {
            members.erase(it);
            break;
        }
    }
    if (isOperator(fd))
        removeOperator(fd);
}

void Channel::addOperator(int fd)
{
    if (!isOperator(fd))
        operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
    for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); ++it)
    {
        if (*it == fd)
        {
            operators.erase(it);
            return;
        }
    }
}

const std::vector<int> &Channel::getMembers() const
{
    return members;
}

void Channel::broadcast(const std::string &message, int senderFd, std::map<int, Client> &clients) const
{
    for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it == senderFd)
            continue;

        std::map<int, Client>::iterator clientIt = clients.find(*it);
        if (clientIt != clients.end())
            clientIt->second.appendOut(message);
    }
}
