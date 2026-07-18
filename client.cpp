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
    clientaddress.sin_port = htons(6667);
    inet_pton(AF_INET, "127.0.0.1", &clientaddress.sin_addr);
    connect(sockfd, (struct sockaddr *)&clientaddress, sizeof(clientaddress));
    int i = 0;
    while (i < 100)
    {
        write(sockfd, "O", 1);
        i++;
        sleep(1);
    }
}
