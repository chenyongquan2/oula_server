#ifndef NET_TCPCLIENT_H
#define NET_TCPCLIENT_H

#include "Eventloop.h"
#include "Connector.h"
#include "utils/InetAddress.h"
#include "utils/nocopyable.h"
#include "Eventloop.h"
#include <memory>
#include <string>
#include "Callback.h"

using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient:nocopyable
{
public:
    TcpClient(EventLoop* eventloop, const InetAddress& serverAddr,
            const std::string& nameArg);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    EventLoop* getLoop() const
        {return loop_;}

    bool isEnableRetry() const 
        {return retry_;}
    //允许与服务端断开链接后重连
    void enableRetry() 
        {retry_ = true;}

    const std::string& name() const
        {return name_;}

    /// Set connection callback.
    /// Not thread safe.
    void setNewConnectionCallback(ConnectionCallback& cb)
        {connectionCallback_ = std::move(cb);}
    
    // Set message callback.
    /// Not thread safe.
    void setMessageCallback(MessageCallback cb)
    { messageCallback_ = std::move(cb); }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    { writeCompleteCallback_ = std::move(cb); }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

private:
    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    bool enableConnect_;
    bool retry_;

    //always in loop thread
    int nextConnId_;
    TcpConnectionPtr connection_;

};

#endif // !NET_TCPCLIENT_H
