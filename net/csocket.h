#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <sys/epoll.h>

#define NGX_MAX_EVENTS 512	  
class Connection;

class Socket
{
public:
    Socket();
    ~Socket();
    void Init();

    bool run();
    void AcceptCallBack(Connection* conn);
    void ClientReadCallBack(Connection* conn);

private:
    bool InitEpoll();
    bool InitListenSocket();

private:
    int m_epollFd;
    epoll_event m_epollEvents[NGX_MAX_EVENTS];
    int m_listenFd;
};

#endif // end NET_SOCKET_H


