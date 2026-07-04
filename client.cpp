#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in clientaddress;
    clientaddress.sin_family = AF_INET;
    clientaddress.sin_port = htons(55555);
    inet_pton(AF_INET, "127.0.0.1", &clientaddress.sin_addr);
    connect(sockfd, (struct sockaddr *)&clientaddress, sizeof(clientaddress));
    write(sockfd, "hell", 4);
}
