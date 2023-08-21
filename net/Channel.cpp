#include "Channel.h"

#include <poll.h>
#include <sys/poll.h>

#include "Eventloop.h"

//POLLPRT它表示有一个或多个高优先级的带外（out-of-band）数据可供读取
// 在网络编程中，常常使用POLLPRI来检测带外数据的到达。带外数据是指在TCP连接中的紧急数据，它具有高优先级，需要被立即处理。通过使用POLLPRI标志位，可以确保程序能够及时响应带外数据的到达。
// 当一个套接字或文件描述符上发生POLLPRI事件时，可以调用适当的函数进行处理。例如，可以使用recv()函数来接收带外数据。
// 总而言之，POLLPRI用于在非阻塞I/O操作中检测和处理带外数据的到达。它是多路复用I/O的一部分，使得程序能够及时响应高优先级的数据。
constexpr int KReadEvent = POLLIN | POLLPRI;
constexpr int KWriteEvent = POLLOUT;
constexpr int KNoneEvent = 0;

Channel::Channel(EventLoop* pEventLoop, int fd)
    : eventloop_(pEventLoop)
    , m_socketFd(fd)
    , events_(KNoneEvent)
{

}
Channel::~Channel()
{

}

void Channel::update()
{
    //通过eventloop_的poller去更新channel的interested events.
    eventloop_->updateChannel(this);
}

int Channel::GetAllEvents()
{
    return events_;
}

void Channel::EnableReadEvent()
{
    events_ |= KReadEvent;
    update();
}

void Channel::DisableReadEvent()
{
    //错误写法：!KReadEvent;
    events_ &= ~KReadEvent; 
    update();
}

void Channel::EnableWriteEvent()
{
    events_ |= KWriteEvent;
    update();
}

void Channel::DisableWriteEvent()
{
    events_ &= ~KWriteEvent;
    update();
}

void Channel::DisableAllEvent()
{
    events_ = KNoneEvent;
    update();
}

bool Channel::IsEnableReadEvent()
{
    return events_ & KReadEvent;
}

bool Channel::IsEnableWriteEvent()
{
    return events_ & KWriteEvent;
}

void Channel::SetReceiveEvent(int revt)
{
    rEvents_ = revt;
}

void Channel::HandleEvent()
{
    if(rEvents_ & POLLIN)
    {
        if(readCallback_)
        {
            readCallback_();
        }
    }
    if(rEvents_ & POLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}

void Channel::SetReadCallback(EventCallback callback)
{
    readCallback_ = callback;
}

void Channel::SetWirteCallback(EventCallback callback)
{
    writeCallback_ = callback;
}

int Channel::GetSocketFd()
{
    return m_socketFd;
}



