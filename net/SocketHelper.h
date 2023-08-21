#ifndef NET_SOCKETHELPER_H
#define NET_SOCKETHELPER_H

#include <netinet/in.h>

namespace SocketHelper {
    int CreateNonblockingOrDie(sa_family_t family);
    void bindOrDie(int sockfd);
    void listenOrDie(int sockfd);
    void setReuseAddr(int sockfd, bool on);
    void setReusePort(int sockfd, bool on);
    int accept(int listenSockfd, struct sockaddr_in* addr);
}

#endif //NET_SOCKETHELPER_H