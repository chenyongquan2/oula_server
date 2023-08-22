
#include "TcpServer.h"
#include "Acceptor.h"
#include "Channel.h"
#include "Eventloop.h"
#include "TcpConnection.h"
#include <functional>
#include <string>
#include "iostream"

TcpServer::TcpServer(EventLoop* eventloop)
    :eventloop_(eventloop)
    ,acceptor_(new Acceptor(eventloop))
    ,nextConnId_(1)
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
    
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(eventloop_,sockfd);
    connections_[std::to_string(nextConnId_)] = conn;
    conn->setMessageCallback(messageCallback_);

    //执行conn的ConnectEstablished方法，将其的chanel给enableReading
    eventloop_->runInLoop(std::bind(&TcpConnection::ConnectEstablished,conn));

    ++nextConnId_;
}
