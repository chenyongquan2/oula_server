#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include <iostream>


// void defaultConnectionCallback(const TcpConnectionPtr& conn)
// {
//     std::cout << "new conn" << std::endl;
// }

TcpConnection::TcpConnection(EventLoop * eventloop, int sockfd)
    :eventloop_(eventloop)
    ,socket_(new Socket(sockfd))
    ,channel_(new Channel(eventloop,sockfd))
{
    channel_->SetReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->SetWirteCallback(std::bind(&TcpConnection::handleWirte, this));
}

void TcpConnection::handleRead()
{
    std::cout << "handleRead" << std::endl;
}

void TcpConnection::handleWirte()
{
    std::cout << "handleWrite" << std::endl;
}