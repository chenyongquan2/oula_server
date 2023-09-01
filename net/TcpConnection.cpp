#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cassert>
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
    std::cout << "ConnectEstablished, and EnableReadEvent" << std::endl;
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
    //size_t nWrote = 0;
    ssize_t nWrote = 0;//it must be signed, bcz nWrote can be -1 when ::write() return
    size_t remaining = len;
    bool errorOccurs = false;

    bool canWriteDirect = !(channel_->IsEnableWriteEvent());
    if(canWriteDirect
        && outputBuffer_.readableBytes() == 0)
    {
        //write the data directly.
        nWrote =::write(channel_->GetSocketFd(), data, len);
        if(nWrote >= 0)
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
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    // EPIPE（Broken Pipe）：发生在向一个已关闭的写端的管道或套接字写入数据时。比如，如果一个进程尝试向一个已经关闭的管道写入数据，那么EPIPE错误将被设置。
                    // ECONNRESET（Connection Reset）：发生在套接字连接被对方重置或关闭时。比如，在TCP连接中，如果对方已经关闭了连接，而本地套接字仍然尝试发送数据，则ECONNRESET错误将被设置。
                    // 这两种错误场景下，errno会被设置为对应的错误码，即EPIPE或ECONNRESET。在代码中通过判断errno的值是否等于这两个错误码，可以根据不同的错误情况采取相应的处理措施。
                    errorOccurs = true;
                    std::cout << "TcpConnection::send fail bcz peer closed!" << std::endl;
                }
            }
        }
    }

    assert(remaining <= len);
    if(!errorOccurs && remaining > 0)
    {
        std::cout << "TcpConnection::send it has more data to send, add writing event to continue!" << std::endl;
        
    }
   
    
}
