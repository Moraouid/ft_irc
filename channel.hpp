#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <string>
#include <vector>

class Client;

class Channel
{
private:
    std::string name;
    std::string topic;
    std::vector<int> members;
    std::vector<int> operators;

public:
    Channel();
    explicit Channel(const std::string &channelName);
    ~Channel();

    const std::string &getName() const;
    const std::string &getTopic() const;
    void setTopic(const std::string &newTopic);

    bool hasMember(int fd) const;
    bool isOperator(int fd) const;

    void addMember(int fd);
    void removeMember(int fd);
    void addOperator(int fd);
    void removeOperator(int fd);

    const std::vector<int> &getMembers() const;
    void broadcast(const std::string &message, int senderFd, std::map<int, Client> &clients) const;
};

#endif
