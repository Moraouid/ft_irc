#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>

// Enum to track where the user is in the authentication flow
enum RegistrationState {
    UNREGISTERED,
    PASS_ACCEPTED,
    NICK_SET,
    REGISTERED
};

class Client {
    private:
        int                 fd;
        std::string         ipAddress;
        
        std::string         nickname;
        std::string         username;
        std::string         realname;
        
        RegistrationState   state;
        std::string         inBuffer;
        std::string         outBuffer;

    public:
        Client();
        Client(int fd, std::string ip);
        Client(const Client &copy);
        Client &operator=(const Client &assign);
        ~Client();

        // Getters
        int                 getFd() const;
        std::string         getNickname() const;
        std::string         getUsername() const;
        RegistrationState   getState() const;
        std::string&        getInBuffer();
        std::string&        getOutBuffer();

        // Setters
        void                setNickname(std::string nick);
        void                setUsername(std::string user);
        void                setState(RegistrationState state);

        void                appendIn(char data[1000], int rd);
        void                appendOut(std::string data);
        void                clearInBuffer();
};

#endif