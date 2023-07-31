#include "csocket.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <cerrno>
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

#include "../threadpool/threadpool.h"
#include "../logic/logic.h"

using namespace std;

bool Socket::run()
{
    int ret=0;
    int port = 3000;
    int epollFdMax = 1024;

    //创建listen socket
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        cout << "create listen socket error." << std::endl;
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

    //按照refactor模式

    //创建epoll，把listenfd给添加到epoll，等待其可读事件
    int epollFd = epoll_create(epollFdMax);//Todo:先随便写个值。这个值是不是没啥用了?
    if (epollFd == -1)
    {
        std::cout << "create epollFd error." << std::endl;
        return false;
    }
    epoll_event epollEvent;
    epollEvent.data.fd = listenFd;
    epollEvent.events = EPOLLIN;
    epollEvent.events |= EPOLLET;//et模式

    ret=epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &epollEvent);
    if(ret==-1)
    {
        std::cout << "epoll_ctl error" << std::endl;
        return false;
    }

    while(true)
    {
        epoll_event epollEvents[epollFdMax];
        int timeout = -1;//暂时先无限等待下去
        //epollEvents是个出参，会把有事件的结果给放到这里。
        int epollEventCnt = epoll_wait(epollFd, epollEvents, epollFdMax, timeout);
        if(epollEventCnt < 0)
        {
            if(errno==EINTR)
            {
                //被信号中断
                continue;
            }
            else
            {
                //出错
                std::cout << "epoll_wait error" << std::endl;
                break;
            }
        }
        else if(epollEventCnt == 0)
        {
            //超时了
            continue;
        }
        else
        {
            for(auto i=0;i<epollEventCnt;i++)
            {
                //读事件
                if(epollEvents[i].events & EPOLLIN)
                {
                    //判断是否为listen fd
                    if(epollEvents[i].data.fd == listenFd)
                    {
                        static size_t clientNo =0;
                        sockaddr_in clientAddr;
                        socklen_t clientAddrLen = sizeof(clientAddr);
                        int clientFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                        if(clientFd ==-1)
                        {
                            std::cout << "accept error" << std::endl;
                            return false;
                        }

                        clientNo++;

                        //客户端连接上了!
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
                            return false;
                        }

                        //把client给添加到epoll
                        epoll_event clientFdEpollEvent;
                        clientFdEpollEvent.data.fd = clientFd;
                        clientFdEpollEvent.events = EPOLLIN;
                        //clientFdEpollEvent.events |= EPOLLET;//开启et
                        ret=epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &clientFdEpollEvent);
                        if(ret == -1)
                        {
                            std::cout << "new client accepted,clientfd: " << clientFd << std::endl;
                            return false;
                        }
                    }
                    else
                    {
                        //为client的可读事件
                        int curClientFd = epollEvents[i].data.fd;
                        //std::cout << "client fd: " << curClientFd << " recv data." << std::endl;
                        char buf[32]={0};
                        int sz=recv(curClientFd,buf,sizeof(buf),0);
                        if(sz==0)
                        {
                            //说明对端关闭了连接
                            //将client从epoll中移除事件
                            ret=epoll_ctl(epollFd, EPOLL_CTL_DEL, curClientFd, nullptr);
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
                                continue;
                            }
                            else
                            {
                                //将client从epoll中移除事件
                                ret=epoll_ctl(epollFd, EPOLL_CTL_DEL, curClientFd, nullptr);
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

                            string param(buf);
                            pThreadPool->addTask(&Logic::Exec, param);
                        }
                        
                    }
                }
                else if(epollEvents[i].events & EPOLLOUT)
                {
                    // TODO 暂不处理
                }
                
            }
        }

    }

    close(listenFd);
    return true;
}


