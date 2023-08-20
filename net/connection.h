#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H


#include "Channel.h"
#include "sockethelper.h"

#include <memory>
#include <iostream>  
#include <tuple>  
#include <utility>
#include <functional>

class EventLoop;
class TcpConnection;

typedef void (*EventCallbackType)(EventLoop*, TcpConnection*);

//Connection层，每个客户端连接代表一个Connection对象，用于记录该路连接的各种状态
//常见的状态信息有，如连接状态、数据收发缓冲区信息、数据流量记录状态、本端和对端地址和端口号信息等，
//同时也提供对各种网络事件的处理接口，
//这些接口或被本层自己使用，或被 Session 层使用。Connection 持有一个 Channel 对象，且掌管着 Channel 对象的生命周期。


class TcpConnection
{
public:
    TcpConnection(int socketFd, EventLoop *pEventLoop);
    virtual ~TcpConnection();

    int GetSockFd();
    void handleReadEvent();

public:
    EventCallbackType m_readEventCallbackFunc;//读事件处理回调
    EventCallbackType m_writeEventCallbackFunc;//写事件处理回调

private:
    EventLoop *m_pEventLoop;
    int m_socketFd;
    std::shared_ptr<Channel> m_spChannel;
    //Todo：本端和对端的地址信息， 接受缓冲区 发送缓冲区 流量统计等信息

};

class ListenConnection: public TcpConnection
{
public:
    ListenConnection(int socketFd, EventLoop *pEventLoop);
    virtual ~ListenConnection() {}
};

class ClientConnection: public TcpConnection
{
public:
    ClientConnection(int socketFd, EventLoop *pEventLoop);
    virtual ~ClientConnection() {}
};

//方便使用依赖注入的设计模式，这里抽象出来一个interface
class TcpConnectionMgrInterface
{
public:
    virtual ~TcpConnectionMgrInterface() {}
    //外部只需要以sockfd为ket就通过TcpConnectionMgr去找到对应的conn，而无需直接和具体的conn去打交道。
    virtual void AddListenConn(int sockFd) = 0;
    virtual void AddClientConn(int sockFd) = 0;
    virtual void RemoveConn(int sockFd) = 0;
    virtual bool HasConn(int sockFd) = 0;
    virtual void HandleRead(int sockFd) = 0;
};

//连接池
class TcpConnectionMgr: public TcpConnectionMgrInterface
{
public:
    virtual ~TcpConnectionMgr();
    TcpConnectionMgr(EventLoop *pEventLoop);
    virtual void AddListenConn(int sockFd) override;
    virtual void AddClientConn(int sockFd) override;
    virtual void RemoveConn(int sockFd) override;
    virtual bool HasConn(int sockFd) override;
    virtual void HandleRead(int sockFd) override;
    
private:
    EventLoop* m_pEventLoop;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> m_connMap;
};

#endif // end NET_CONNECTION_H