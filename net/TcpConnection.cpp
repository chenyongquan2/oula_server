#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "Buffer.h"
#include "TcpServer.h"


// void defaultConnectionCallback(const TcpConnectionPtr& conn)
// {
//     std::cout << "new conn" << std::endl;
// }

TcpConnection::TcpConnection(EventLoop * eventloop, int sockfd, std::string& name, const InetAddress& localAddr, const InetAddress& peerAddr)
    :eventloop_(eventloop)
    ,socket_(new Socket(sockfd))
    ,channel_(new Channel(eventloop,sockfd))
    ,name_(name)
    ,localAddr_(localAddr)
    ,peerAddr_(peerAddr)
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
    //强调此函数只能在eventloop_所绑定的线程中被调用
    eventloop_->assertInLoopThread();
    channel_->EnableReadEvent();
}

void TcpConnection::ConnectDestoryed()
{
    eventloop_->assertInLoopThread();
    channel_->DisableAllEvent();
    channel_->remove();//Todo: what's the diff between remove and DisableAllEvent?
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

void TcpConnection::send(const std::string &message)
{
    int len = message.length();
    return send(message.data(), len);

}

void TcpConnection::send(const void* data, size_t len)
{
    size_t nWrote = 0;
    size_t remaining = len;

    if(outputBuffer_.readableBytes() == 0)
    {
        //write the data directly.
        nWrote =::write(channel_->GetSocketFd(), data, len);
        if(nWrote >= 0)//Todo:can be == 0 ?
        {
            remaining -= nWrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                //eventloop_->runInLoop(std::bind(writeCompleteCallback_), shared_from_this());
            }
        }
        else if(nWrote < 0)
        {
            if(errno != EWOULDBLOCK)
            {

            }
        }
    }
    
}
