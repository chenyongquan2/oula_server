#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H



#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include "Acceptor.h"
#include "Callback.h"
#include "Buffer.h"
#include "string.h"
#include "utils/InetAddress.h"

class EventLoop;
class Channel;
class Socket;
class TcpConnection;

void defaultConnectionCallback(const TcpConnectionPtr& conn);

class TcpConnection :public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop * eventloop, int sockfd, std::string& name, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return eventloop_; }
    const std::string& name() const 
        {return name_;}

    void send(const std::string &message);
    void send(const void* data, size_t len);
    
    //callback
    void setMessageCallback(const MessageCallback& cb)
        {messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback&cb)
        {writeCompleteCallback_ = cb;}

    void setCloseCallback(const CloseCallback& cb)//notice arg cb must be a right value.
        {closeCallback_ = cb;}
    
    //it will called when the conn established
    void ConnectEstablished();
    //it will called when the conn destoryed
    void ConnectDestoryed();


    //optional function
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

private:
    //chanel会根据事件类型，执行TcpConnection层对应类型handle函数
    void handleRead();
    void handleWirte();
    void handleClose();



private:
    EventLoop* eventloop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    //conn event callback
    //NewConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    //
    WriteCompleteCallback writeCompleteCallback_;

    //when the conn closed, it will call this func, to remove the conn in the Tcpserver's conn map
    CloseCallback closeCallback_;


    //buffer
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    //prop
    std::string name_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
 
};

#endif //NET_TCPCONNECTION_H
