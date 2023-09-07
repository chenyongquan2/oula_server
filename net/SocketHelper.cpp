#include "SocketHelper.h"
#include <asm-generic/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

static void _SetNonBlockingAndCloseOnExec(int sockfd)
{
    //non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret  = ::fcntl(sockfd, F_SETFL, flags);
    if(ret < 0)
    {

    }

    //FD_CLOEXEC
    //FD_CLOEXEC 标志表示在执行 exec() 系统调用时，该文件描述符将会被关闭，以避免在子进程中意外继承该文件描述符。
    flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    ret  = ::fcntl(sockfd, F_SETFL, flags);

    if(ret < 0)
    {

    }
}

//ASSERT
// 通过将这些声明放在命名空间中，可以避免全局命名冲突的问题。
// 在头文件中声明命名空间 MyNamespace，然后在实现文件中使用 namespace MyNamespace 来实现命名空间中的函数。
// 这样做的目的是为了确保实现文件中的函数定义与头文件中的声明一致，并将函数定义置于正确的命名空间中。
// 如果不在实现文件中使用 namespace MyNamespace，那么实现的函数将不会属于任何命名空间（重要!）
namespace SocketHelper {

int CreateNonblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd<0)
    {

    }
    _SetNonBlockingAndCloseOnExec(sockfd);

    return sockfd;
}

void bindOrDie(int sockfd, const struct sockaddr* addr)
{

    int ret = ::bind(sockfd, (struct sockaddr *)addr,sizeof(*addr));
    if(ret <0)
    {

    }
}

void listenOrDie(int sockfd)
{
    //SOMAXCONN 操作系统默认的最大连接数
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret <0)
    {

    }
}

void setReuseAddr(int sockfd, bool on)
{
    //设置ip和地址可重用，防止timewait状态存在导致端口无法复用。
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(ret==-1)
    {
        
    }
}

void setReusePort(int sockfd, bool on)
{
    //设置ip和地址可重用，防止timewait状态存在导致端口无法复用。
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));   
    if(ret==-1)
    {
        
    }
}

int accept(int listenSockfd, struct sockaddr_in* addr)
{
    sockaddr_in connAddr;
    socklen_t len = sizeof(connAddr);
    int connfd = ::accept(listenSockfd, (struct sockaddr *)&connAddr , &len);
    _SetNonBlockingAndCloseOnExec(connfd);

    //Todo:check error?

    return connfd;
}

struct sockaddr_in getLocalAddr(int sockfd)
{
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    struct sockaddr* pAddr = sockaddr_cast(&addr);
    socklen_t addlen = static_cast<socklen_t>(sizeof(addr));
    if(::getsockname(sockfd, pAddr, &addlen) < 0)
    {

    }
    return addr;
}

struct sockaddr_in getPeerAddr(int sockfd)
{
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    struct sockaddr* pAddr = sockaddr_cast(&addr);
    socklen_t addlen = static_cast<socklen_t>(sizeof(addr));
    if(::getpeername(sockfd, pAddr, &addlen) < 0)
    {

    }
    return addr;
}

struct sockaddr* sockaddr_cast(struct sockaddr_in * addr)
{
    struct sockaddr* pAddr = static_cast<struct sockaddr*>((void *)(addr));
    return pAddr;
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in * addr)
{
    const struct sockaddr* pAddr = static_cast<const struct sockaddr*>((void *)(addr));
    return pAddr;
}

int getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optlen));
    if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

bool isSelfConnect(int sockfd)
{
    struct sockaddr_in localAddr = getLocalAddr(sockfd);
    struct sockaddr_in peerAddr =getPeerAddr(sockfd);

    // 自连接通常是在开发和测试过程中出现的，用于模拟客户端与服务器之间的通信。它可以用于在本地主机上测试网络应用程序的正确性和可靠性，而无需真正与远程服务器进行通信。
    // 自连接的典型示例是在本地主机上运行一个服务器程序，并使用相同的主机地址和端口号作为客户端程序来连接该服务器。这样，服务器程序实际上会与自己建立连接，模拟出客户端和服务器之间的通信。
    // 需要注意的是，自连接只在本地主机上有效，因为在网络中，每个主机都有唯一的IP地址。对于不同主机上的进程或应用程序，它们不能通过相同的IP地址和端口号进行自连接，因为它们位于不同的主机上。
    bool bSamePort = localAddr.sin_port == peerAddr.sin_port;
    bool bSameAddr = localAddr.sin_addr.s_addr == peerAddr.sin_addr.s_addr;
    return bSamePort && bSameAddr;
}

}
