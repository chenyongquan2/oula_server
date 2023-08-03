#include "sockethelper.h"
#include "eventloop.h"
#include "connection.h"
#include "../threadpool/threadpool.h"
#include "../logic/logic.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <error.h>

namespace SocketHelper 
{

bool InitListenSocket(EventLoop* pEventLoop)
{
    constexpr int port = 3000;
    int ret = -1;
    //创建listen socket
    //Todo:后续socket的生命周期感觉得统一交给chanel层来管理
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        std::cout << "create listen socket error." << std::endl;
        return false;
    }

    //设置为非阻塞
    int flags = fcntl(listenFd, F_GETFL,0);
    flags |= O_NONBLOCK;
    ret = fcntl(listenFd, F_SETFL, flags);
    if(ret==-1)
    {
        std::cout << "fcntl set socket block fail" << std::endl;
        return false;
    }

    //设置ip和地址可重用，防止timewait状态存在导致端口无法复用。
    int reuseAddr = 1;
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));
    if(ret==-1)
    {
        std::cout << "setsockopt fail" << std::endl;
        return false;
    }

    int reusePort = 1;  
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));   
    if(ret==-1)
    {
        std::cout << "setsockopt fail" << std::endl;
        return false;
    }

    //上述操作得在bind前去做。
    //bind绑定
    sockaddr_in bindaddr;
    bindaddr.sin_family=AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(port);
    ret=bind(listenFd, (const sockaddr*)&bindaddr, sizeof(bindaddr));
    if(ret==-1)
    {
        std::cout << "bind listen socket error." << std::endl;
        return false;
    }

    //启动listen
    ret = listen(listenFd, SOMAXCONN);
    if(ret==-1)
    {
        std::cout << "listen error." << std::endl;
        return false;
    }

    //创建conn
    pEventLoop->GetConnectionMgr()->AddListenConn(listenFd);

    epoll_event epollEvent;
    epollEvent.data.fd = listenFd;
    epollEvent.events = EPOLLIN;
    //epollEvent.events |= EPOLLET;//et模式
    //epollEvent.data.ptr = spListenConn;//方便epoll触发事件驱动时，去拿到对应的conn

    ret = epoll_ctl(pEventLoop->GetEpollFd(), EPOLL_CTL_ADD, listenFd, &epollEvent);
    if(ret==-1)
    {
        std::cout << "epoll_ctl error" << std::endl;
        return false;
    }

    return true;
}

void AcceptConn(EventLoop* pEventLoop, TcpConnection* pListenConn)
{
    int ret = -1;
    static size_t clientNo =0;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    //Todo:后续socket的生命周期感觉得统一交给chanel层来管理
    int clientFd = accept(pListenConn->GetSockFd(), (struct sockaddr *)&clientAddr, &clientAddrLen);
    if(clientFd ==-1)
    {
        std::cout << "accept error" << std::endl;
        return;
    }

    //客户端连接上了!
    clientNo++;
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, sizeof(clientIP));
    int clientPort=ntohs(clientAddr.sin_port);
    std::cout << "Accepted clientNo:" << clientNo << ", connection from " << clientIP << ":" << clientPort << std::endl;  

    //设置clientFd为非阻塞
    int flags = fcntl(clientFd, F_GETFL,0);
    flags |= O_NONBLOCK;
    ret = fcntl(clientFd, F_SETFL, flags);
    if(ret == -1)
    {
        std::cout << "set clientfd to nonblocking error." << std::endl;
        return;
    }

    //创建conn
    pEventLoop->GetConnectionMgr()->AddClientConn(clientFd);

    //把client给添加到epoll
    epoll_event clientFdEpollEvent;
    clientFdEpollEvent.data.fd = clientFd;
    clientFdEpollEvent.events = EPOLLIN;
    //clientFdEpollEvent.events |= EPOLLET;//开启et
    //clientFdEpollEvent.data.ptr= spClientConn;

    ret = epoll_ctl(pEventLoop->GetEpollFd(), EPOLL_CTL_ADD, clientFd, &clientFdEpollEvent);
    if(ret == -1)
    {
        std::cout << "new client accepted,clientfd: " << clientFd << std::endl;
        return;
    }
}

void ClientReadCallBack(EventLoop* pEventLoop, TcpConnection* pClientConn)
{
    int ret = -1;
    //为client的可读事件
    int curClientFd = pClientConn->GetSockFd();
    //std::cout << "client fd: " << curClientFd << " recv data." << std::endl;
    char buf[32]={0};
    int sz=recv(curClientFd,buf,sizeof(buf),0);
    if(sz==0)
    {
        //说明对端关闭了连接
        //将client从epoll中移除事件
        ret=epoll_ctl(pEventLoop->GetEpollFd(), EPOLL_CTL_DEL, curClientFd, nullptr);
        if(ret==-1)
        {
            std::cout << "EPOLL_CTL_DEL clientfd fail:" << curClientFd << std::endl;
        }
        std::cout << "client disconnected, clientfd:" << curClientFd << std::endl;

        //关闭clientFd
        close(curClientFd);
    }
    else if(sz<0) 
    {
        if(errno==EINTR //被信号中断了
            || errno==EWOULDBLOCK //暂时没数据
            || errno==EAGAIN)
        {
            return;
        }
        else
        {
            //将client从epoll中移除事件
            ret=epoll_ctl(pEventLoop->GetEpollFd(), EPOLL_CTL_DEL, curClientFd, nullptr);
            if(ret==-1)
            {
                std::cout << "EPOLL_CTL_DEL clientfd fail:" << curClientFd << std::endl;
            }
        }

        std::cout << "recv data error." << std::endl;
    }
    else
    {
        //正常收到数据
        //std::cout << "recv from client:" << curClientFd << buf << std::endl;

        //交由消费者(线程池)去消费
        auto pThreadPool = ThreadPool::GetInstance();
        std::string param(buf);
        pThreadPool->addTask(&Logic::Exec, param);
    }
}

void CloseSocket(int sockFd)
{
    //Todo:后续socket的生命周期感觉得统一交给chanel层来管理
    close(sockFd);
}

};
