#include "TcpClient.h"
#include "Callback.h"
#include "Connector.h"
#include "Eventloop.h"
#include "SocketHelper.h"
#include "TcpConnection.h"
#include <memory>
#include "utils/InetAddress.h"
#include "utils/log.h"

namespace details
{
void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(
        &TcpConnection::ConnectDestoryed,conn
    ));
}

};

TcpClient::TcpClient(EventLoop* eventloop, const InetAddress& serverAddr,
        const std::string& nameArg)
        :loop_(eventloop)
        ,name_(nameArg)
        ,connector_(std::make_shared<Connector>(loop_, serverAddr))
        ,connectionCallback_(defaultConnectionCallback)
        ,messageCallback_(defaultMessageCallback)
        ,retry_(false)
        ,enableConnect_(true)
        ,nextConnId_(0)
        
{
    //connector_里有个channel，channel有个socket,放到了poller里面，当客户端有连接上来后回自动调用TcpClient::newConnection
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1)
    );
    Logger::GetInstance()->debug("TcpClient::TcpClient[{}]- connector" ,name_);
}

TcpClient::~TcpClient()
{
    Logger::GetInstance()->debug("TcpClient::~TcpClient[{}]- connector" ,name_);
    TcpConnectionPtr conn;
    bool isUnique = false;
    {
        isUnique = connection_.unique();
        conn = connection_;
    }
    if(conn)
    {
        assert(loop_ == conn->getLoop());
        //not thread safe
        //其实就是想调用TcpConnection::ConnectDestoryed
        //注意这里的类型:
        //typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
        //本来void removeConnection(EventLoop* loop, TcpConnectionPtr& conn);是需要两个参数
        //这里用std::bind()过程顺便给第一个参数确定为了loop,这样只要填第二个参数即可。
        //相当于变成了void removeConnection(已确定不用再填空, TcpConnectionPtr& conn);
        CloseCallback cb = std::bind(&details::removeConnection, loop_, std::placeholders::_1);
        
        //这里本来的类型为void setCloseCallback(const CloseCallback& cb)
        //但是这里std::bind(...,cb)手动绑定了cb作为参数了，这样std::bind()返回的functional相当于少了一个变量
        //变成了void(固定值cb)的类型,完美符合了runInLoop要求的参数类型为 std::function<void()> Functor;
        loop_->runInLoop(
            std::bind(&TcpConnection::setCloseCallback,conn,cb)
        );

        if(isUnique)
        {
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
    }
}

void TcpClient::connect()
{
    Logger::GetInstance()->debug("TcpClient::connect[{}]- connecting to" ,connector_->serverAddress().toIpPort());
    enableConnect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    enableConnect_ = false;
    //Todo::lock it?
    if (connection_)
    {
      connection_->shutdown();
    }
}

void TcpClient::stop()
{
    
}

void TcpClient::newConnection(int sockfd)
{
    //当客户端有连接上来后回自动调用TcpClient::newConnection
    loop_->assertInLoopThread();
    ++nextConnId_;

    InetAddress localAddr(SocketHelper::getLocalAddr(sockfd));
    InetAddress peerAddr(SocketHelper::getPeerAddr(sockfd));
    std::string name = std::to_string(nextConnId_);

    //这里会由accrptor给回调到此。
    Logger::GetInstance()->debug("TcpClient::newConnection sockfd: {}, connId:{},localAddr ip:{},port:{},peerAddr ip:{},port:{}" , 
        sockfd, nextConnId_ ,localAddr.toIp(), localAddr.port(), peerAddr.toIp(), peerAddr.port());


    TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, sockfd, name, localAddr, peerAddr);
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)
    );

    {
        //Todo:上锁?
        //赋值，让当前client拥有connection的智能指针。
        connection_ = conn;
    }

    //由于client只有一个eventloop，不用太考虑多个eventloop的多线程场景。
    conn->ConnectEstablished();
    //ioLoop->runInLoop(std::bind(&TcpConnection::ConnectEstablished,conn));
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        //Todo:上锁?
        assert(connection_ == conn);
        //Todo:上锁?
        connection_.reset();//智能指针reset
    }

    loop_->queueInLoop(std::bind(&TcpConnection::ConnectDestoryed, conn));

    if(retry_ && enableConnect_)
    {
        Logger::GetInstance()->debug("TcpClient::connect name:{} - Reconnecting to {}", name_, connector_->serverAddress().toIpPort());
        connector_->restart();
    }
}