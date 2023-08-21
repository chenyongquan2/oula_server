#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include "Channel.h"
#include <sys/epoll.h>
#include <functional>
#include <memory>

class TcpConnection;
class TcpConnectionMgrInterface;

class Poller;

class EventLoop
{
public:
    typedef std::function<void()> Functor;
public:
    EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop) = delete;
    ~EventLoop();

    void loop();
    void runInLoop(Functor cb);

    //管理channel相关
    void updateChannel(Channel *);
    void removeChannel(Channel *);
    bool HasChannel(Channel *);

private:
    bool quit_;
    std::unique_ptr<Poller> poller_;

    typedef std::vector<Channel*> ChannelList;
    ChannelList activeChanels_;
};

#endif // end NET_EVENTLOOP_H


