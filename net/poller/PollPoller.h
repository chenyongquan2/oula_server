#ifndef NET_POLLER_POLLPOLLER_H
#define NET_POLLER_POLLPOLLER_H

#include "../Poller.h"
#include <vector>

class PollPoller: public Poller
{
public:
    PollPoller(EventLoop *loop);
    virtual ~PollPoller();

    virtual void poll(int timeoutMs, ChannelList* activeChannels) override;

    //把一个channel给加入poll监听
    virtual void updateChannel(Channel*) override;
    //把一个channel给移除poll监听
    virtual void RemoveChannel(Channel*) override;

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChanels) const;

private:
    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollfds_;
    std::unordered_map<int,int> fdInpollfdsIdx_;
};

#endif // !NET_POLLER_POLLPOLLER_H
