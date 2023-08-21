#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H



#include <functional>
#include <memory>
class EventLoop;
class Channel;
class Socket;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
void defaultConnectionCallback(const TcpConnectionPtr& conn);

class TcpConnection
{
public:
    TcpConnection(EventLoop * eventloop, int sockfd);


private:
    //chanel会根据事件类型，执行TcpConnection层对应类型handle函数
    void handleRead();
    void handleWirte();

private:
    

private:
    EventLoop* eventloop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
 
};

#endif //NET_TCPCONNECTION_H
