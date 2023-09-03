#include "Socket.h"
#include "SocketHelper.h"  
#include "utils/InetAddress.h"
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <netinet/tcp.h>


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

void Socket::bindAddress(const InetAddress& listenAddr)
{
    SocketHelper::bindOrDie(sockfd_, listenAddr.getSockAddr());
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

void Socket::setTcpNoDelay(bool on)
{
    int optVal = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,&optVal, static_cast<socklen_t>(sizeof(optVal)));
}

void Socket::setKeepAlive(bool on)
{
    int optVal = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optVal, static_cast<socklen_t>(sizeof(optVal)));
}