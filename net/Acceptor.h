#ifndef NET_ACCEEPTOR_H
#define NET_ACCEEPTOR_H

#include "memory"
#include <functional>
#include <memory>
#include "Callback.h"


class EventLoop;
class Socket;
class Channel;
class InetAddress;


class Acceptor
{
public:
    
public:
    Acceptor(EventLoop* eventloop, const InetAddress& listenAddr);
    ~Acceptor();

    void listen();
    bool isListening();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_  = cb;
    }

private:
    void HandleRead();

private:
    EventLoop* eventloop_;
    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel> acceptChannel_; 
    NewConnectionCallback newConnectionCallback_;

    int listenFd_;
    bool isListening_;
};

#endif //NET_ACCEEPTOR_H