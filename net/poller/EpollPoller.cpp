#include "EpollPoller.h"
#include "../Eventloop.h"
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

static constexpr int KInitEventListSize = 16;

EpollPoller::EpollPoller(EventLoop* eventloop)
    :Poller(eventloop)
    ,events_(KInitEventListSize)
{
    // FD_CLOEXEC 标志表示在执行 exec() 系统调用时，该文件描述符将会被关闭，以避免在子进程中意外继承该文件描述符。
    // 通过在 epoll_create1() 中设置 EPOLL_CLOEXEC 标志，可以确保创建的 epoll 实例在执行 exec() 系统调用时会被自动关闭。这在编写多进程或多线程程序时非常有用，可以避免在执行 exec() 时意外继承 epoll 实例的文件描述符。
    epollfd_ = epoll_create1(EPOLL_CLOEXEC);

}
EpollPoller::~EpollPoller()
{
// ::是作用域解析运算符，用于指定调用全局命名空间中的函数或变量。
// 在这种情况下，::close() 表示调用全局命名空间中的 close() 函数，
// 而不是可能存在于当前命名空间或其他命名空间中的同名函数。这是一种显式地指定要使用的函数的方式，以避免命名冲突或歧义。
    ::close(epollfd_);
}

void EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)  
{
    int numEvents = ::epoll_wait(epollfd_, &(*events_.begin()), events_.size(), timeoutMs);
    //Todo:errno==EINTR?
    if(numEvents>0)
    {
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==events_.size())
        {
            //自动扩容成2倍
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents==0)
    {

    }
    else if(numEvents<0)
    {

    }

}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChanels) const
{
    for(int i=0;i<numEvents;i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->SetReceiveEvent(events_[i].events);
        activeChanels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel)  
{
    //Todo: how about EPOLL_CTL_MOD?
    updateOperator2Poller(EPOLL_CTL_ADD, channel);

}
void EpollPoller::RemoveChannel(Channel*channel)  
{
    updateOperator2Poller(EPOLL_CTL_DEL, channel);
}

void EpollPoller::updateOperator2Poller(int operation, Channel*channel)
{
    struct epoll_event event;
    //Todo:复习一下这个函数写法
    memset(&events_, 0, sizeof(event));
    event.data.ptr=channel;
    event.events=channel->GetAllEvents();//感兴趣的事件。

    int fd=channel->GetSocketFd();
    int ret=epoll_ctl(epollfd_, operation, fd, &event);
    if(ret<0)
    {

    }
}

