#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <sys/epoll.h>
#include <memory>

#define NGX_MAX_EVENTS 512

class TcpConnection;
class TcpConnectionMgrInterface;

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
    void Init();

    TcpConnectionMgrInterface* GetConnectionMgr() const;
    int GetEpollFd() const;
    bool processEventLoop();

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


