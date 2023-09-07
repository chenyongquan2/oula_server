
#include "TcpServer.h"
#include "Acceptor.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include "Eventloop.h"
#include "SocketHelper.h"
#include "TcpConnection.h"
#include <cstddef>
#include <functional>
#include <string>
#include "iostream"
#include "Callback.h"
#include "utils/InetAddress.h"
#include "utils/log.h"

TcpServer::TcpServer(EventLoop* eventloop, const InetAddress& listenAddr)
    :eventloop_(eventloop)
    ,acceptor_(new Acceptor(eventloop, listenAddr))
    ,nextConnId_(1)
    ,connectionCallback_(defaultConnectionCallback)
    ,messageCallback_(defaultMessageCallback)//默认读事件处理函数。
    ,threadPool_(new EventLoopThreadPool(eventloop, "oula"))
{

    //当acceptor accpet了新连接之后，需要调用 TcpServer::newConnection，来把conn给放到连接池里面管理。
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1)
    );

    //default callback, optional
    writeCompleteCallback_ =  defaultWriteCompleteCallback;
    highWaterMarkCallback_ = defaultHighWaterMarkCallback;
    highWaterMark_ = (64*1024);//1024
}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
    //start the trheadpool's thread to eventloop
    threadPool_->start(threadInitCallback_);//即使这里是个空实现，底层有兜底。

    assert(!acceptor_->isListening());

    //通知acceptor去执行listen
    eventloop_->runInLoop(
        std::bind(&Acceptor::listen, acceptor_.get())
    );
}

void TcpServer::setThreadNum(size_t numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd)
{
    eventloop_->assertInLoopThread();

    //round robin
    EventLoop *ioLoop = threadPool_->getNextLoop();

    InetAddress localAddr(SocketHelper::getLocalAddr(sockfd));
    InetAddress peerAddr(SocketHelper::getPeerAddr(sockfd));
    std::string name = std::to_string(nextConnId_);

    //这里会由accrptor给回调到此。
    Logger::GetInstance()->debug("TcpServer::newConnection sockfd: {}, connId:{},localAddr ip:{},port:{},peerAddr ip:{},port:{}" , 
        sockfd, nextConnId_ ,localAddr.toIp(), localAddr.port(), peerAddr.toIp(), peerAddr.port());
    
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, sockfd, name, localAddr, peerAddr);
    connections_[name] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setHighWaterMarkCallback(highWaterMarkCallback_, highWaterMark_);

    //notice:std::bind 创建的临时函数对象,是一个右值,
    //所以setCloseCallback的参数得是右值，void setCloseCallback(const CloseCallback& cb)
    //而不能为void setCloseCallback(const CloseCallback& cb)
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    //执行conn的ConnectEstablished方法，将其的chanel给enableReading
    ioLoop->runInLoop(std::bind(&TcpConnection::ConnectEstablished,conn));
    //eventloop_->runInLoop(std::bind(&TcpConnection::ConnectEstablished,conn));

    ++nextConnId_;
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    //bcz it will called by the other thread if the conn disconn in other ioloop instead of base main loop.
    eventloop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    eventloop_->assertInLoopThread();

    EventLoop *ioLoop = conn->getLoop();

    const std::string& name = conn->name();
    assert(connections_.find(name)!=connections_.end());
    connections_.erase(name);

    ioLoop->queueInLoop(std::bind(&TcpConnection::ConnectDestoryed, conn));
}
