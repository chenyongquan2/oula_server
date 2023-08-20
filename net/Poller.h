#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <map>
#include <vector>
#include "Channel.h"
class EventLoop;

class Poller
{
public:
    typedef std::vector<Channel*> ChannelList;
public:
    static Poller* NewDefaultPoller(EventLoop *loop);

    Poller(EventLoop *loop);
    virtual ~Poller();

    virtual void poll(int timeoutMs, ChannelList* activeChannels) =0;

    virtual void AddChannel(Channel*) = 0;
    virtual void RemoveChannel(Channel*) = 0;
    virtual bool HasChannel(Channel*) = 0;

protected:
    //key为socket Fd
    typedef std::map<int, Channel*> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop* eventloop_;
};

#endif //NET_POLLER_H