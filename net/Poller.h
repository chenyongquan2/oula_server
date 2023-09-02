#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <map>
#include <vector>
#include "Channel.h"
#include "Eventloop.h"

class Poller
{
public:
    typedef std::vector<Channel*> ChannelList;
public:
    static Poller* NewDefaultPoller(EventLoop *loop);

    Poller(EventLoop *loop);
    virtual ~Poller();

    virtual void poll(int timeoutMs, ChannelList* activeChannels) =0;

    virtual void updateChannel(Channel*) = 0;
    virtual void RemoveChannel(Channel*) = 0;


    bool HasChannel(Channel*);

    void assertInLoopThread() const
    {
        return eventloop_->assertInLoopThread();
    }

protected:
    //key为socket Fd
    typedef std::map<int, Channel*> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop* eventloop_;
};

#endif //NET_POLLER_H