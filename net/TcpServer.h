#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H


#include "Acceptor.h"
#include "Callback.h"
#include "EventLoopThreadPool.h"
#include "utils/InetAddress.h"
#include <functional>
#include <memory>
#include <map>
#include <string>
#include <functional>
#include "TcpConnection.h"


class EventLoop;
class Acceptor;

class TcpServer
{
public:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
public:
    TcpServer(EventLoop* eventloop, const InetAddress& listenAddr);
    ~TcpServer();

    void start();
    void setMessageCallback(const MessageCallback& cb)
        {messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {writeCompleteCallback_ = cb;}

    
    //one loop one thread
    std::shared_ptr<EventLoopThreadPool> threadPool()
        { return threadPool_; }
    void setThreadNum(size_t numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb)
        { threadInitCallback_ = cb; }

private:
    void newConnection(int sockfd);//, const InetAddress& peerAddr
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    EventLoop* eventloop_; // the acceptor loop.
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionMap connections_;
    int nextConnId_;

    //one loop one thread
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    //user set callback
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
};



#endif //NET_TCPSERVER_H