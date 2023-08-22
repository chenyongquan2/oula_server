#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H


#include "Acceptor.h"
#include "Callback.h"
#include "utils/InetAddress.h"
#include <functional>
#include <memory>
#include <map>
#include <string>
#include "TcpConnection.h"
class EventLoop;
class Acceptor;


class TcpServer
{
public:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
public:
    TcpServer(EventLoop* eventloop);//Todo:const InetAddress& listenAddr
    ~TcpServer();

    void start();
    void setMessageCallback(MessageCallback& cb)
        {messageCallback_ = cb;}

private:
    void newConnection(int sockfd);//, const InetAddress& peerAddr

private:
    EventLoop* eventloop_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionMap connections_;
    int nextConnId_;

    //user set callback
    MessageCallback messageCallback_;
};



#endif //NET_TCPSERVER_H