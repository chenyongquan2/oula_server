
#include "TcpServer.h"
#include "Acceptor.h"
#include "Channel.h"
#include "Eventloop.h"
#include "TcpConnection.h"
#include <functional>
#include <string>
#include "iostream"
#include "Callback.h"

TcpServer::TcpServer(EventLoop* eventloop)
    :eventloop_(eventloop)
    ,acceptor_(new Acceptor(eventloop))
    ,nextConnId_(1)
    ,messageCallback_(defaultMessageCallback)//默认读事件处理函数。
{

    //当acceptor accpet了新连接之后，需要调用 TcpServer::newConnection，来把conn给放到连接池里面管理。
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1)
    );
}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
    //通知acceptor去执行listen
    eventloop_->runInLoop(
        std::bind(&Acceptor::listen, acceptor_.get())
    );
}

void TcpServer::newConnection(int sockfd)
{
    //这里会由accrptor给回调到此。
    std::cout << "TcpServer::newConnection sockfd:" << sockfd << 
        "connId:" << nextConnId_ << std::endl;
    
    std::string name = std::to_string(nextConnId_);
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(eventloop_,sockfd, name);
    connections_[name] = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //notice:std::bind 创建的临时函数对象,是一个右值,
    //所以setCloseCallback的参数得是右值，void setCloseCallback(const CloseCallback& cb)
    //而不能为void setCloseCallback(const CloseCallback& cb)
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    //执行conn的ConnectEstablished方法，将其的chanel给enableReading
    eventloop_->runInLoop(std::bind(&TcpConnection::ConnectEstablished,conn));

    ++nextConnId_;
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    const std::string& name = conn->name();
    assert(connections_.find(name)!=connections_.end());
    connections_.erase(name);


}
