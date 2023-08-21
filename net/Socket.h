#ifndef NET_SOCKET_H
#define NET_SOCKET_H

class EventLoop;
class TcpConnection;

class Socket
{
public:
    explicit Socket(int sockfd);   
    ~Socket();

    int fd();
    
    void bindAddress();
    void listen();

    //only if the listen socket can call this function
    int accept();


private:
    int sockfd_;
};



#endif //NET_SOCKET_H