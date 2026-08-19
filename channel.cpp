#include "channel.hpp"
#include "client.hpp"

Channel::Channel() : name(""), topic("default topic"), isPrivate(false), userLimit(0), hasLimit(false) {}

Channel::Channel(const std::string &channelName) : name(channelName), topic("default topic"), isPrivate(false), userLimit(0), hasLimit(false) {}

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

void Channel::setPrivate(bool isPrivate)
{
    this->isPrivate = isPrivate;
}

void Channel::setLimit(int limit)
{
    if (limit > 0)
    {
        userLimit = limit;
        hasLimit = true;
    }
}

void Channel::removeLimit()
{
    userLimit = 0;
    hasLimit = false;
}

bool Channel::hasUserLimit() const
{
    return hasLimit;
}

int Channel::getUserLimit() const
{
    return userLimit;
}

bool Channel::getIsPrivate() const
{
    return isPrivate;
}

bool Channel::isInvited(int fd) const
{
    return std::find(invitedMembers.begin(), invitedMembers.end(), fd) != invitedMembers.end();
}

void Channel::addInvitedMember(int fd)
{
    if (!hasMember(fd) && !isInvited(fd))
        invitedMembers.push_back(fd);
}

void Channel::removeInvitedMember(int fd)
{
    for (std::vector<int>::iterator it = invitedMembers.begin(); it != invitedMembers.end(); ++it)
    {
        if (*it == fd)
        {
            invitedMembers.erase(it);
            return;
        }
    }
}

const std::vector<int> &Channel::getInvitedMembers() const
{
    return invitedMembers;
}
