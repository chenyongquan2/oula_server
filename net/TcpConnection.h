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
private:
     enum StateE 
    {
        KConnecting = 0,//正在建立连接中
        KConnected,//已经建立完成连接
        KDisconnecting,//正在关闭连接
        KDisconnected,//已经关闭连接
    };
public:
    TcpConnection(EventLoop * eventloop, int sockfd, std::string& name, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return eventloop_; }
    const std::string& name() const 
        {return name_;}

    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool isConnected() 
    {
        return state_ == KConnected;
    }
    bool isDisconnected()
    {
        return state_ == KDisconnected;
    }

    void send(const std::string &message);
    void send(const void* data, size_t len);
    
    //user callback
    void setConnectionCallback(const ConnectionCallback& cb)
        {connectionCallback_ = cb;}

    void setMessageCallback(const MessageCallback& cb)
        {messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback&cb)
        {writeCompleteCallback_ = cb;}

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    void setCloseCallback(const CloseCallback& cb)//notice arg cb must be a right value.
        {closeCallback_ = cb;}
    
    //it will called when the conn established
    void ConnectEstablished();
    //it will called when the conn destoryed
    void ConnectDestoryed();

    //some close methods
    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);

    //optional function
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

private:
    //chanel会根据事件类型，执行TcpConnection层对应类型handle函数
    void handleRead();
    void handleWirte();
    void handleClose();

    void sendInLoop(const std::string &message);
    void sendInLoop(const void* data, size_t len);

    const char* stateToString() const;
    void setState(StateE s) { state_ = s; }

    //关闭写端。
    void shutdownInLoop();
    //force close really
    void forceCloseInLoop();

private:
    EventLoop* eventloop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    //conn event callback
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    //when all the data sended
    WriteCompleteCallback writeCompleteCallback_;

    //when the write data more then highWaterMark_, it will be called
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    //when the conn closed, it will call this func, to remove the conn in the Tcpserver's conn map
    CloseCallback closeCallback_;



    //buffer
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    //prop
    std::string name_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    //conn state.
    StateE state_;//Todo:should be atomic.
 
};

#endif //NET_TCPCONNECTION_H
