#include "Poller.h"
#include "poller/EpollPoller.h"


Poller* Poller::NewDefaultPoller(EventLoop *loop)
{
    //Todo:暂时全部返回epoll
    return new EpollPoller(loop);
}

Poller::Poller(EventLoop *loop)
    :eventloop_(loop)
{

}

Poller::~Poller()
{

}

bool Poller::HasChannel(Channel* channel)
{
    auto it = channels_.find(channel->GetSocketFd());
    if(it == channels_.end())
    {
        return false;
    }
    return it->second == channel;
}

