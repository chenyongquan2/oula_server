#include "InetAddress.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include "../SocketHelper.h"

// INADDR_ANY代表任意IPv4地址，它的值是0.0.0.0。
// 在服务器端调用bind函数时，如果将IP地址设置为INADDR_ANY，表示服务器可以接受任意IP地址的连接请求，即监听所有可用的网络接口。
// 当在客户端调用connect函数时，如果将目标IP地址设置为INADDR_ANY，表示连接到本机的任意IP地址。

// INADDR_LOOPBACK代表回环地址，它的值是127.0.0.1。
// 回环地址是一个虚拟的网络接口，用于在本机内部进行通信，一般用于本机上的进程之间进行通信。
// 当在服务器端调用bind函数时，如果将IP地址设置为INADDR_LOOPBACK，表示服务器只接受来自本机的连接请求。
// 当在客户端调用connect函数时，如果将目标IP地址设置为INADDR_LOOPBACK，表示连接到本机上的某个服务。
// 总结来说，INADDR_ANY表示任意IPv4地址，可以用于服务器端监听所有可用的网络接口；而INADDR_LOOPBACK表示回环地址，用于本机内部进程之间的通信。

static constexpr in_addr_t KInetAny = INADDR_ANY;
static constexpr in_addr_t KInetLoopback = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = KInetAny;
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    ::inet_pton(AF_INET, ip.c_str(), &(addr_.sin_addr));
}

uint16_t InetAddress::port() const
{
    uint16_t port = ::ntohs(addr_.sin_port);
    return port;
}

std::string InetAddress::toIp() const
{
    char buf[64] = "";
    if(addr_.sin_family == AF_INET)
    {
        //p 表示 "presentation"，即表示要转换的 IP 地址的字符串形式。而 n 则表示 "network"，即表示要存储结果的目标内存地址
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    }
    
    return buf;
}

std::string InetAddress::toIpPort() const
{
    //Todo:后面这里可以尝试改造为fmt库来实现。
    std::string buf;
    buf.append(toIp());
    buf.append(":");
    buf.append(std::to_string(port()));
    return buf;
}

const struct sockaddr* InetAddress::getSockAddr() const
{
    return SocketHelper::sockaddr_cast(&addr_);
}