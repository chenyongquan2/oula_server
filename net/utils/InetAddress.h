#ifndef NET_UTILS_INETADDRESS_H
#define NET_UTILS_INETADDRESS_H

#include "nocopyable.h"
#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

class InetAddress
{
public:
    InetAddress() = default;
    ~InetAddress() = default;

    explicit InetAddress(const struct sockaddr_in& addr)
        :addr_(addr){}

    InetAddress(uint16_t port);
    InetAddress(const std::string& ip, uint16_t port);

    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    uint16_t port() const;
    std::string toIpPort() const;

    const struct sockaddr* getSockAddr() const;
    void setSockAddr(const struct sockaddr_in& addr) { addr_ = addr; }

private:
    struct sockaddr_in addr_;
};

#endif //NET_UTILS_INETADDRESS_H