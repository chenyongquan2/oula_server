#include "Eventloop.h"
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
#include "Poller.h"
#include <unistd.h>

const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    :quit_(false)
{
    init();
}

void EventLoop::init()
{
    poller_.reset(Poller::NewDefaultPoller(this));

}

EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{
    std::cout<<"EventLoop start"<<std::endl;
    while(!quit_)
    {
        activeChanels_.clear();
        poller_->poll(kPollTimeMs,&activeChanels_);

        for(auto channel:activeChanels_)
        {
            channel->HandleEvent();
        }
    }
}

void EventLoop::runInLoop(Functor cb)
{
    
}

void EventLoop::updateChannel(Channel *channel)
{
    //转调poller，因为poller是来监听channel的事件。
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel *channel)
{
    return poller_->HasChannel(channel);
}



