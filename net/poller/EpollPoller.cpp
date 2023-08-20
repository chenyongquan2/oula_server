#include "EpollPoller.h"
#include "../Eventloop.h"

EpollPoller::EpollPoller(EventLoop* eventloop)
    :Poller(eventloop)
{

}
EpollPoller::~EpollPoller()
{

}

void EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)  
{

}

void EpollPoller::AddChannel(Channel*)  
{

}
void EpollPoller::RemoveChannel(Channel*)  
{

}
bool EpollPoller::HasChannel(Channel*)  
{
    
}