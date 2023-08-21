#ifndef NET_UTILS_INETADDRESS_H
#define NET_UTILS_INETADDRESS_H

#include <bits/stdint-uintn.h>
#include <netinet/in.h>
#include <sys/socket.h>
class InetAddress
{
public:
    explicit InetAddress(const struct sockaddr_in& addr)
        :addr_(addr){}

    sa_family_t family() const {return addr_.sin_family;}
    
private:
    struct sockaddr_in addr_;
};

#endif //NET_UTILS_INETADDRESS_H