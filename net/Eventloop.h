#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "Channel.h"
#include <sys/epoll.h>
#include <functional>
#include <memory>

#define NGX_MAX_EVENTS 512

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
    
    
private:
    bool quit_;
    std::unique_ptr<Poller> poller_;

    typedef std::vector<Channel*> ChannelList;
    ChannelList activeChanels_;

private:
    bool InitEpoll();
    void InitConnMgr();
    bool InitListenSocket();

private:
    std::shared_ptr<TcpConnectionMgrInterface> m_spTcpConnectionMgr;
    int m_epollFd;
    epoll_event m_epollEvents[NGX_MAX_EVENTS];
};

#endif // end NET_SOCKET_H


