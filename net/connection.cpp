#include "connection.h"
#include "Eventloop.h"
#include "sockethelper.h"

#include <iostream>
#include <memory>

TcpConnection::TcpConnection(int socketFd, EventLoop *pEventLoop)
    :m_socketFd(socketFd)
    ,m_pEventLoop(pEventLoop)
{
    m_spChannel = std::make_shared<Channel>(socketFd, m_pEventLoop);
}

TcpConnection::~TcpConnection()
{
    //Todo:这里的socket的生命周期后续改造为channel层来管理。
    SocketHelper::CloseSocket(m_socketFd);
}

int TcpConnection::GetSockFd()
{
    return m_socketFd;
}

void TcpConnection::handleReadEvent()
{
    EventCallbackType pFunc = m_readEventCallbackFunc;
    (*pFunc)(m_pEventLoop, this);
}

ListenConnection::ListenConnection(int socketFd, EventLoop *pEventLoop)
    :TcpConnection(socketFd, pEventLoop)
{
    m_readEventCallbackFunc = &SocketHelper::AcceptConn;
    //Todo:write callback
}


ClientConnection::ClientConnection(int socketFd, EventLoop *pEventLoop)
    :TcpConnection(socketFd, pEventLoop)
{
    m_readEventCallbackFunc = &SocketHelper::ClientReadCallBack;
    //Todo:write callback
}


///////////////////////////////////////////////////////
//TcpConnectionMgr
TcpConnectionMgr::TcpConnectionMgr(EventLoop *pEventLoop)
    :m_pEventLoop(pEventLoop)
{

}

TcpConnectionMgr::~TcpConnectionMgr()
{
    m_connMap.clear();
}

void TcpConnectionMgr::AddListenConn(int sockFd)
{
    m_connMap.emplace(sockFd, std::make_shared<ListenConnection>(sockFd, m_pEventLoop));
}

void TcpConnectionMgr::AddClientConn(int sockFd)
{
    m_connMap.emplace(sockFd, std::make_shared<ClientConnection>(sockFd, m_pEventLoop));
}

void TcpConnectionMgr::RemoveConn(int sockFd)
{
    m_connMap.erase(sockFd);
}

bool TcpConnectionMgr::HasConn(int sockFd)
{
    return m_connMap.find(sockFd) != m_connMap.end();
}

void TcpConnectionMgr::HandleRead(int sockFd)
{
    bool bExist = HasConn(sockFd);
    if(!bExist)
    {
        std::cout << "TcpConnectionMgr::HandleRead conn not exist!" <<std::endl;
        return;
    }
    auto pConn = m_connMap.at(sockFd);
    pConn->handleReadEvent();
}





