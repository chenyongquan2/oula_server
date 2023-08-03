#include "eventloop.h"
#include "connection.h"
#include "sockethelper.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <error.h>

EventLoop::EventLoop()
    :m_epollFd(-1)
{
    Init();
}

EventLoop::~EventLoop()
{

}

void EventLoop::Init()
{
    InitEpoll();
    InitConnMgr();
    SocketHelper::InitListenSocket(this);
}

void EventLoop::InitConnMgr()
{
    m_spTcpConnectionMgr = std::make_shared<TcpConnectionMgr>(this);
}

bool EventLoop::InitEpoll()
{
    constexpr int epollFdMax = 1024;
    //创建epoll，把listenfd给添加到epoll，等待其可读事件
    m_epollFd = epoll_create(epollFdMax);//Todo:先随便写个值。这个值是不是没啥用了?
    if (m_epollFd == -1)
    {
        std::cout << "create epollFd error." << std::endl;
        return false;
    }
    return true;
}

TcpConnectionMgrInterface* EventLoop::GetConnectionMgr() const
{
    return m_spTcpConnectionMgr.get();
}

int EventLoop::GetEpollFd() const
{
    return m_epollFd;
}

bool EventLoop::processEventLoop()
{
    int ret=0;
    //按照refactor模式
    while(true)
    {
        int timeout = -1;//暂时先无限等待下去
        //epollEvents是个出参，会把有事件的结果给放到这里。
        int epollEventCnt = epoll_wait(m_epollFd, m_epollEvents, NGX_MAX_EVENTS, timeout);
        if(epollEventCnt < 0)
        {
            if(errno==EINTR)
            {
                //被信号中断
                continue;
            }
            else
            {
                //出错
                std::cout << "epoll_wait error:" << errno << std::endl;
                break;
            }
        }
        else if(epollEventCnt == 0)
        {
            //超时了
            continue;
        }
        else
        {
            for(auto i=0;i<epollEventCnt;i++)
            {
                if(m_epollEvents[i].events & EPOLLIN)
                {
                    int fd = m_epollEvents[i].data.fd;
                    //这里用了依赖注入的设计思想，它通过将依赖的对象从外部传入，而不是在内部创建或查找依赖，从而实现了组件的解耦和可重用性
                    //这里不需要管fd是啥类型，connMgr内部会处理。
                    m_spTcpConnectionMgr->HandleRead(fd);
                }
                else if(m_epollEvents[i].events & EPOLLOUT)
                {
                    // TODO 暂不处理
                }
            }
        }
    }
    
    return true;
}


