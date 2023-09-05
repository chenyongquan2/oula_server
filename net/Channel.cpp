#include "Channel.h"

#include <cassert>
#include <poll.h>
#include <sstream>
#include <sys/poll.h>
#include <iostream>

#include "Eventloop.h"
#include "utils/log.h"

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
    , eventHandling_(false)
    , inPollerStatus_(ChannelInPollerStatus_KInit)
{

}
Channel::~Channel()
{
    assert(!eventHandling_);
}

void Channel::update()
{
    //通过eventloop_的poller去更新channel的interested events.
    eventloop_->updateChannel(this);
}

void Channel::remove()
{
    //remove the channel's event in the event loop
    eventloop_->removeChannel(this);
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

 bool Channel::IsNoneEvent()
 {
    return events_ == KNoneEvent;
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
    eventHandling_ = true;
    Logger::GetInstance()->debug(reventsToString());

    if((rEvents_ & POLLHUP) && !(rEvents_ & POLLIN))
    {
        //POLLHUP 的全称是 "Poll Hang Up"，它是基于 poll 函数的一个事件标志。
        //POLLHUP 用于检测文件描述符上的挂起（hang up）事件。
        //当与文件描述符关联的连接被远程对等方关闭时，会触发 POLLHUP 事件。
        //当与文件描述符关联的连接发生错误或异常情况时，也可能触发 POLLHUP 事件
        //需要注意的是，POLLHUP 事件可能与其他事件同时发生。因此，在处理事件时，通常需要综合考虑其他事件标志，例如 POLLIN（可读事件）或 POLLOUT（可写事件）。
        //具体的触发条件和处理方式可能会因操作系统、编程语言或应用场景而有所不同。
        Logger::GetInstance()->debug("fd = {} Channel::handle_event() POLLHUP", sockFd_);
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    
    if(rEvents_ & (POLLIN | POLLPRI|  POLLRDHUP))
    {
        //POLLRDHUP 的全称是 "Poll Read Hang Up"，它是基于 poll 函数的一个事件标志。POLLRDHUP 用于检测文件描述符上的半关闭（half-close）事件
        //需要注意的是，POLLRDHUP 事件可能与其他事件同时发生

        //POLLPRI 的英文全称是 "Poll Priority"，它是基于 poll 函数的一个事件标志。POLLPRI 用于检测紧急数据的到达，通常与带外数据（out-of-band data）相关。
        //POLLPRI 事件可能与其他事件同时发生，因此在处理事件时通常需要综合考虑其他事件标志，
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
    eventHandling_ = false;
}

int Channel::GetSocketFd()
{
    return m_socketFd;
}

std::string Channel::reventsToString() const
{
  return eventsToString(m_socketFd, rEvents_);
}

std::string Channel::eventsToString() const
{
  return eventsToString(m_socketFd, events_);
}

std::string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << "sockFd:" << fd << ",events happen:";
    if (ev & POLLIN)
    oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}




