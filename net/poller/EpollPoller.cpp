#include "EpollPoller.h"
#include "../Eventloop.h"
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <cassert>
#include "../utils/log.h"

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
    Poller::assertInLoopThread();
    Channel::ChannelInPollerStatus inPollerStatus = channel->getInPollerStatus();
    int fd = channel->GetSocketFd();
    Logger::GetInstance()->debug( "fd = {}, events = {}, index = {} ", fd, channel->GetAllEvents(), inPollerStatus);

    if(inPollerStatus == Channel::ChannelInPollerStatus_KInit 
        || inPollerStatus == Channel::ChannelInPollerStatus_KDeleted)
    {
        
        if(inPollerStatus == Channel::ChannelInPollerStatus_KInit)
        {
            channels_[fd] = channel;
        }
        else if(inPollerStatus == Channel::ChannelInPollerStatus_KDeleted)
        {
            //is existing channels in channels_
            assert(channels_.find(fd)!=channels_.end());
            assert(channels_[fd]==channel);
        }

        //update one with EPOLL_CTL_ADD
        channel->setInPollerStatus(Channel::ChannelInPollerStatus_KAdded);
        updateOperator2Poller(EPOLL_CTL_ADD, channel);
        
    }
    else if(inPollerStatus == Channel::ChannelInPollerStatus_KAdded)
    {
        //update existing one with EPOLL_CTL_MOD/DEL
        assert(channels_.find(fd)!=channels_.end());
        assert(channels_[fd]==channel);
        if(channel->IsNoneEvent())
        {
            updateOperator2Poller(EPOLL_CTL_DEL, channel);
            channel->setInPollerStatus(Channel::ChannelInPollerStatus_KDeleted);
        }
        else
        {
            updateOperator2Poller(EPOLL_CTL_MOD, channel);
        }
    }
    
}
void EpollPoller::RemoveChannel(Channel*channel)  
{   
    //when this will be called ? 
    //when: channel_->remove() -> EventLoop::removeChannel ->this
    //remove the channel's event in the event loop

    Poller::assertInLoopThread();
    int fd = channel->GetSocketFd();
    Logger::GetInstance()->debug("RemoveChannel fd {} from epoller", fd);

    //is must existing in the channels_
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());

    Channel::ChannelInPollerStatus inPollerStatus = channel->getInPollerStatus();
    assert(inPollerStatus == Channel::ChannelInPollerStatus_KAdded
        || inPollerStatus == Channel::ChannelInPollerStatus_KDeleted);
    
    channels_.erase(fd);

    if(inPollerStatus == Channel::ChannelInPollerStatus_KAdded)
    {
        //unregister from the poller
        updateOperator2Poller(EPOLL_CTL_DEL, channel);
    }

    channel->setInPollerStatus(Channel::ChannelInPollerStatus_KInit);
}

void EpollPoller::updateOperator2Poller(int operation, Channel*channel)
{
    struct epoll_event event;
    //Todo:复习一下这个函数写法
    memset(&event, 0, sizeof(event));
    event.data.ptr=channel;
    event.events=channel->GetAllEvents();//感兴趣的事件。

    int fd=channel->GetSocketFd();
    Logger::GetInstance()->debug("epoll_ctl op = {}, fd = {},event = {} ",operationToString(operation),fd, channel->eventsToString());

    int ret=epoll_ctl(epollfd_, operation, fd, &event);
    if(ret<0)
    {
        Logger::GetInstance()->error("EpollPoller::updateOperator2Poller epoll_ctl failed!");
    }
}

const char* EpollPoller::operationToString(int op)
{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}

