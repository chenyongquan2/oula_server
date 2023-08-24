#ifndef NET_SOCKETHELPER_H
#define NET_SOCKETHELPER_H

#include <netinet/in.h>
#include <sys/socket.h>

namespace SocketHelper {
    int CreateNonblockingOrDie(sa_family_t family);
    void bindOrDie(int sockfd, const struct sockaddr* addr);
    void listenOrDie(int sockfd);
    void setReuseAddr(int sockfd, bool on);
    void setReusePort(int sockfd, bool on);
    int accept(int listenSockfd, struct sockaddr_in* addr);

    //获取套接字（socket）的本地地址和远程地址的函数
    struct sockaddr_in getLocalAddr(int sockfd);
    struct sockaddr_in getPeerAddr(int sockfd);

    //convert func.
    // sockaddr_in在sockaddr的基础上增加了sin_port和sin_addr字段，分别用于表示端口号和IPv4地址。
    // 而sockaddr只是一个通用的结构体，没有专门用于表示端口号和IPv4地址的字段。
    // 在实际编程中，当需要使用IPv4地址和端口号时，通常使用sockaddr_in结构体表示。而sockaddr在一些接口函数中使用，用于通用的地址表示。
    struct sockaddr* sockaddr_cast(struct sockaddr_in * addr);
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in * addr);
}

#endif //NET_SOCKETHELPER_H