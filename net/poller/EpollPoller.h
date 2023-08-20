#ifndef NET_POLLER_EPOLLPOLLER_H
#define NET_POLLER_EPOLLPOLLER_H

#include "../Poller.h"

class EpollPoller:public Poller
{
public:
    EpollPoller(EventLoop* eventloop);
    virtual ~EpollPoller();

    virtual void poll(int timeoutMs, ChannelList* activeChannels) override;

    virtual void AddChannel(Channel*) override;
    virtual void RemoveChannel(Channel*) override;
    virtual bool HasChannel(Channel*) override;

};

#endif //NET_POLLER_EPOLLPOLLER_H