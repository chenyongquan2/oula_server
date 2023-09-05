#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "Buffer.h"
#include "TcpServer.h"
#include "utils/log.h"

TcpConnection::TcpConnection(EventLoop * eventloop, int sockfd, std::string& name, const InetAddress& localAddr, const InetAddress& peerAddr)
    :eventloop_(eventloop)
    ,socket_(new Socket(sockfd))
    ,channel_(new Channel(eventloop,sockfd))
    ,name_(name)
    ,localAddr_(localAddr)
    ,peerAddr_(peerAddr)
    ,state_(KConnecting)//init state.
{
    channel_->SetReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->SetWirteCallback(std::bind(&TcpConnection::handleWirte, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::handleClose,this));

    socket_->setTcpNoDelay(true);
}

TcpConnection::~TcpConnection()
{
    Logger::GetInstance()->debug("TcpConnection::dtor {}, at fd={}, state={}", name_, channel_->GetSocketFd(), stateToString());
    assert(state_ == KDisconnected);
}

const char* TcpConnection::stateToString() const
{
  switch (state_)
  {
    case KDisconnected:
      return "kDisconnected";
    case KConnecting:
      return "kConnecting";
    case KConnected:
      return "kConnected";
    case KDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::ConnectEstablished()
{
    //强调此函数只能在eventloop_所绑定的线程中被调用
    eventloop_->assertInLoopThread();

    assert(state_ == KConnecting);
    setState(KConnected);
    
    channel_->EnableReadEvent();
    Logger::GetInstance()->debug("ConnectEstablished, and EnableReadEvent");
}

void TcpConnection::ConnectDestoryed()
{
    eventloop_->assertInLoopThread();
    if(state_ == KConnected)
    {
        setState(KDisconnected);
        //当前还在KConnected连接状态，得从channels_列表里移除出去。
        //DisableAllEvent()主要是把事件给poller给EPOLL_CTL_DEL，但是还在channels_列表里。
        channel_->DisableAllEvent();
    }
    // what's the diff between remove and DisableAllEvent?
    // DisableAllEvent()主要是把事件给poller给EPOLL_CTL_DEL，但是还在channels_列表里。
    // remove则会把chanel从poller的channels_列表里面移除出去。

    channel_->remove();
}

void TcpConnection::handleRead()
{
    Logger::GetInstance()->debug("handleRead");
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
    Logger::GetInstance()->debug("handleWrite begin");
    eventloop_->assertInLoopThread();
    if(channel_->IsEnableWriteEvent())
    {
        ssize_t n = ::write(channel_->GetSocketFd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if(n>0)
        {
            outputBuffer_.retrieve(n);//pop data
            if(outputBuffer_.readableBytes() == 0)
            {
                //finish to send all the remain data, disbale writing events.
                channel_->DisableWriteEvent();

                if(writeCompleteCallback_)
                {
                    eventloop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }

                if(state_ == KDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            Logger::GetInstance()->debug("handleWrite to send non data");
        }
    }
    else
    {
        //poller通知可写但是却又write不出去。。。
        Logger::GetInstance()->debug("[handleWrite] conn fd:{},is down , no more writing ",channel_->GetSocketFd());
    }
}

void TcpConnection::handleClose()
{
    eventloop_->assertInLoopThread();
    Logger::GetInstance()->debug("fd = {}, state = {}" , channel_->GetSocketFd(), stateToString());
    assert(state_ == KConnected || state_ == KDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(KDisconnected);



    //unregister from poller
    channel_->DisableAllEvent();
    //Todo:this will unregister from poller,how to fix ti.
    //channel_->remove();

    //call TcpServer::removeConnection to remove the conn from the map.
    closeCallback_(shared_from_this());
    
}

void TcpConnection::send(const std::string &message)
{
    size_t len = message.length();
    return send(message.data(), len);

}

void TcpConnection::send(const void* data, size_t len)
{
    if(state_ == KConnected)
    {
        if(eventloop_->isInLoopThread())
        {
            sendInLoop(data, len);
        }
        else
        {
            //Todo:为啥写法2会编译报错，而写法1不会呢？
            //写法1
            void (TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
            eventloop_->runInLoop(
                std::bind(fp, this, data, len)
            );
            //写法2
            // eventloop_->runInLoop(
            //     std::bind(&TcpConnection::sendInLoop, this, data, len);
            // );
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message)
{
    size_t len = message.length();
    sendInLoop(message.data(), len);
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    eventloop_->assertInLoopThread();
    if(state_== KDisconnected)
    {
        Logger::GetInstance()->debug( "disconnected, give up writing");
        return;
    }

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
                //it will call user/tcpserver 's writeCompleteCallback_
                eventloop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
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
                    Logger::GetInstance()->debug("TcpConnection::send fail bcz peer closed!");
                }
            }
        }
    }

    assert(remaining <= len);
    if(!errorOccurs && remaining > 0)
    {   
        //if()
        {
            //判断用户发送缓冲区是否超过了指定的大小，只在上升沿的时候触发一次(第一次超过水位)
            //说明可能用户clinet侧已经接受不过来了。
            size_t oldLen = outputBuffer_.readableBytes();
            if(oldLen+remaining >= highWaterMark_
                && oldLen<highWaterMark_)
            {
                if(highWaterMarkCallback_)
                eventloop_->queueInLoop(std::bind(
                    highWaterMarkCallback_,shared_from_this(), oldLen + remaining
                ));
            }
        }
         
        outputBuffer_.append(static_cast<const char*>(data)+nWrote, remaining);
        if(!channel_->IsEnableWriteEvent())
        {
            Logger::GetInstance()->debug("TcpConnection::send it still has more {} data to send!", remaining);
            channel_->EnableWriteEvent();
        }
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::setKeepAlive(bool on)
{
    socket_->setKeepAlive(on);
}

void TcpConnection::shutdown()
{
    if(state_ == KConnected)
    {
        setState(KDisconnecting);
        eventloop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    eventloop_->assertInLoopThread();
    if(!channel_->IsEnableWriteEvent())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if(state_ == KConnected || state_ == KDisconnecting)
    {
        setState(KDisconnecting);
        eventloop_->queueInLoop(std::bind(
            &TcpConnection::forceCloseInLoop, shared_from_this()
        ));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds)
{
    if(state_ == KConnected || state_ == KDisconnecting)
    {
        setState(KDisconnecting);
        eventloop_->runAfter(seconds, std::bind(
            &TcpConnection::forceCloseInLoop, shared_from_this()
        ));
    }
}

void TcpConnection::forceCloseInLoop()
{
    eventloop_->assertInLoopThread();
    if(state_ == KConnected || state_ == KDisconnecting)
    {
        handleClose();
    }
}
