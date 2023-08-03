#include "connection.h"
#include "csocket.h"
#include <iostream>

int Connection::GetSockFd()
{
    return m_socketFd;
}

ListenConnection::ListenConnection(int socketFd)
    :Connection(socketFd)
{
    m_readEventCallbackFunc = &Socket::AcceptCallBack;
    //Todo:write callback
}


ClientConnection::ClientConnection(int socketFd)
    :Connection(socketFd)
{
    m_readEventCallbackFunc = &Socket::ClientReadCallBack;
    //Todo:write callback
}

