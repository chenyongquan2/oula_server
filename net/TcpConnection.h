#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H



#include <functional>
#include <memory>
#include "Callback.h"
class EventLoop;
class Channel;
class Socket;
class TcpConnection;

void defaultConnectionCallback(const TcpConnectionPtr& conn);

class TcpConnection
{
public:
    TcpConnection(EventLoop * eventloop, int sockfd);
    ~TcpConnection();

    void setMessageCallback(MessageCallback& cb)
        {messageCallback_ = cb;}
    
    //每一个conn建立之后会调用此函数。
    void ConnectEstablished();

private:
    //chanel会根据事件类型，执行TcpConnection层对应类型handle函数
    void handleRead();
    void handleWirte();

private:
    

private:
    EventLoop* eventloop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    //conn event callback
    //NewConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
 
};

#endif //NET_TCPCONNECTION_H
