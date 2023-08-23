#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include <cstddef>
#include <iostream>
#include "Buffer.h"
#include "TcpServer.h"


// void defaultConnectionCallback(const TcpConnectionPtr& conn)
// {
//     std::cout << "new conn" << std::endl;
// }

TcpConnection::TcpConnection(EventLoop * eventloop, int sockfd, std::string& name)
    :eventloop_(eventloop)
    ,socket_(new Socket(sockfd))
    ,channel_(new Channel(eventloop,sockfd))
    ,name_(name)
{
    channel_->SetReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->SetWirteCallback(std::bind(&TcpConnection::handleWirte, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::handleClose,this));
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::ConnectEstablished()
{
    channel_->EnableReadEvent();
}

void TcpConnection::handleRead()
{
    std::cout << "handleRead" << std::endl;
    //read the data from socket ,save to the buffer space;
    size_t n = inputBuffer_.readFd(channel_->GetSocketFd());
    if(n>0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        //error
    }
}

void TcpConnection::handleWirte()
{
    std::cout << "handleWrite" << std::endl;
}

void TcpConnection::handleClose()
{
    //unregister from poller
    channel_->DisableAllEvent();
    //Todo:this will unregister from poller,how to fix ti.
    channel_->remove();

    //call TcpServer::removeConnection to remove the conn from the map.
    closeCallback_(shared_from_this());
    
}
