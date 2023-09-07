#ifndef NET_SOCKET_H
#define NET_SOCKET_H

class EventLoop;
class TcpConnection;
class InetAddress;

class Socket
{
public:
    explicit Socket(int sockfd);   
    ~Socket();

    int fd();
    
    void bindAddress(const InetAddress& listenAddr);
    void listen();

    //only if the listen socket can call this function
    int accept(InetAddress* peeraddr);

    //enable/diable TCP_NODELAY(Nagle's algorithm)
    void setTcpNoDelay(bool on);
    //enable/diable SO_KEEPALIVE
    void setKeepAlive(bool on);

    //关闭写端。
    void shutdownWrite();

private:
    int sockfd_;
};



#endif //NET_SOCKET_H