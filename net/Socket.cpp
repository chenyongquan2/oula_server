#include "Socket.h"
#include "SocketHelper.h"  
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>



Socket::Socket(int sockfd)
    :sockfd_(sockfd)
{

}

Socket::~Socket()
{
    ::close(sockfd_);
}

int Socket::fd()
{
    return sockfd_;
}

void Socket::bindAddress()
{
    SocketHelper::bindOrDie(sockfd_);
}

void Socket::listen()
{
    SocketHelper::listenOrDie(sockfd_);
}

int Socket::accept()
{
    struct sockaddr_in addr;
    int connfd =  SocketHelper::accept(sockfd_, &addr);
    return connfd;
}