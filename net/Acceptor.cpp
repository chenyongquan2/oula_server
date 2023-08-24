#include "Acceptor.h"
#include "Channel.h"
#include "Socket.h"
#include "SocketHelper.h"
#include <memory>
#include <unistd.h>
#include <netinet/in.h>
#include "utils/InetAddress.h"

Acceptor::Acceptor(EventLoop* eventloop, const InetAddress& listenAddr)
    :eventloop_(eventloop)
    ,isListening_(false)
{
    int sockfd = SocketHelper::CreateNonblockingOrDie(AF_INET);

    //Todo:得再在bind之前，设置端口和地址复用。
    SocketHelper::setReuseAddr(sockfd, true);
    SocketHelper::setReusePort(sockfd, true);

    acceptSocket_ = std::make_unique<Socket>(sockfd);
    acceptSocket_->bindAddress(listenAddr);

    acceptChannel_ = std::make_unique<Channel>(eventloop_, acceptSocket_->fd());
    acceptChannel_->SetReadCallback(std::bind(&Acceptor::HandleRead,this));

}

Acceptor::~Acceptor()
{

}


bool Acceptor::isListening()
{
    return isListening_;
}

void Acceptor::listen()
{
    if(isListening_)
        return;
    isListening_=true;
    acceptSocket_->listen();
    acceptChannel_->EnableReadEvent();
}

void Acceptor::HandleRead()
{
    //accept the connect

    int connfd = acceptSocket_->accept();
    if(connfd>=0)
    {
        if(newConnectionCallback_)
        {
            //转调到TcpServer或者TcpClient的::newConnection
            newConnectionCallback_(connfd);
        }
    }
    else
    {
        
    }
}