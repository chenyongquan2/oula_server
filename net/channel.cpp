#include "channel.h"
#include <sys/epoll.h>

Channel::Channel(int fd, EventLoop* pEventLoop)
    : m_socketFd(fd)
    , m_pEventLoop(pEventLoop)
    , m_curEvents(0)
{

}
Channel::~Channel()
{

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

