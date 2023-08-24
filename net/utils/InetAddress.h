#ifndef NET_UTILS_INETADDRESS_H
#define NET_UTILS_INETADDRESS_H

#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

class InetAddress
{
public:
    explicit InetAddress(const struct sockaddr_in& addr)
        :addr_(addr){}

    InetAddress(uint16_t port);
    InetAddress(const std::string& ip, uint16_t port);

    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    uint16_t port() const;

    const struct sockaddr* getSockAddr() const;

private:
    struct sockaddr_in addr_;
};

#endif //NET_UTILS_INETADDRESS_H