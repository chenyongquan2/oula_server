#ifndef NET_POLLER_EPOLLPOLLER_H
#define NET_POLLER_EPOLLPOLLER_H

#include "../Poller.h"
#include <sys/epoll.h>

class EpollPoller: public Poller
{
public:
    EpollPoller(EventLoop* eventloop);
    virtual ~EpollPoller();

    virtual void poll(int timeoutMs, ChannelList* activeChannels) override;

    //把一个channel给加入poll监听
    virtual void updateChannel(Channel*) override;
    //把一个channel给移除poll监听
    virtual void RemoveChannel(Channel*) override;
    

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChanels) const;
    void updateOperator2Poller(int operation, Channel*channel);

private:
    int epollfd_;
    //epoll_wait返回的事件列表。
    typedef std::vector<struct epoll_event> EventList;
    EventList events_;
};

#endif //NET_POLLER_EPOLLPOLLER_H