#include "Channel.h"
#include <poll.h>
#include <sys/poll.h>

Channel::Channel(int fd, EventLoop* pEventLoop)
    : m_socketFd(fd)
    , eventloop_(pEventLoop)
    , m_curEvents(0)
{

}
Channel::~Channel()
{

}

void Channel::GetInterestEvent()
{

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

uint32_t Channel::GetCurEvents()
{
    return m_curEvents;
}

void Channel::SetRecvEvents(uint32_t recvEnents)
{
    m_recvEnents = recvEnents;
}

void Channel::setEnableReadEvent(bool bEable)
{
    if(bEable)
        m_curEvents |= EPOLLIN;
    else
        m_curEvents &= ~EPOLLIN;//& (0)
}

void Channel::setEnableWriteEvent(bool bEable)
{
    if(bEable)
        m_curEvents |= EPOLLOUT;
    else
        m_curEvents &= ~EPOLLOUT;//& (0)
}

