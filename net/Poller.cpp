#include "Poller.h"
#include "poller/EpollPoller.h"
#include "poller/PollPoller.h"

#define POLLER_NAME "Poll"
#define EPOLL_NAME "Epoll"


Poller* Poller::NewDefaultPoller(EventLoop *loop)
{
    const char *pollerType = ::getenv("USE_POLL_TYPE");
    if(pollerType != nullptr 
        && std::string(pollerType) == POLLER_NAME)
    {
        return new PollPoller(loop);
    }
    else 
    {
        return new EpollPoller(loop);
    }
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

