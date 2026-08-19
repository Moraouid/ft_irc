#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <string>
#include <vector>
#include <algorithm>

class Client;

class Channel
{
private:
    std::string name;
    std::string topic;
    std::vector<int> members;
    std::vector<int> operators;
    std::vector<int> invitedMembers;
    bool isPrivate;
    int userLimit;
    bool hasLimit;

public:
    Channel();
    explicit Channel(const std::string &channelName);
    ~Channel();

    const std::string &getName() const;
    const std::string &getTopic() const;
    void setTopic(const std::string &newTopic);

    bool hasMember(int fd) const;
    bool isOperator(int fd) const;
    bool getIsPrivate() const;
    bool isInvited(int fd) const;

    void addMember(int fd);
    void removeMember(int fd);
    void addOperator(int fd);
    void removeOperator(int fd);
    void setPrivate(bool isPrivate);
    void setLimit(int limit);
    void removeLimit();
    bool hasUserLimit() const;
    int getUserLimit() const;
    void addInvitedMember(int fd);
    void removeInvitedMember(int fd);

    const std::vector<int> &getMembers() const;
    const std::vector<int> &getInvitedMembers() const;
    void broadcast(const std::string &message, int senderFd, std::map<int, Client> &clients) const;
};

#endif
