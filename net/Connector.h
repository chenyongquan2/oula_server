#ifndef NET_CONNECTOR_H
#define NET_CONNECTOR_H

#include "memory"
#include <functional>
#include <memory>
#include "Callback.h"
#include "utils/nocopyable.h"
#include "utils/InetAddress.h"


class EventLoop;
class Socket;
class Channel;
class InetAddress;


class Connector :nocopyable, 
    public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void (int sockfd)> NewConnectionCallback;
private:
    enum States { kDisconnected, kConnecting, kConnected };
    
public:
    Connector(EventLoop* eventloop, const InetAddress& serverAddr);
    ~Connector();

    void start();// can be called in any thread
    void restart();// must be called in loop thread
    void stop();// can be called in any thread

    //只有acceptor/conector上的新连接函数才带有new前缀。
    void setNewConnectionCallback(const NewConnectionCallback& cb)
        { newConnectionCallback_  = cb; }

    const InetAddress& serverAddress() const { return serverAddr_; }

private:
    void setState(States s) { state_ = s; }

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);

    //channel's callback function
    void handleWrite();
    void handleError();

    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

private:
    EventLoop* eventloop_;
    InetAddress serverAddr_;
    //标记是否要激活connect
    bool enableConnect_;
    States state_;
    int retryDelayMs_;
    std::unique_ptr<Channel> channel_; 
    NewConnectionCallback newConnectionCallback_;


};

#endif //NET_CONNECTOR_H