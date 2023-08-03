#ifndef NET_SOCKETHELPER_H
#define NET_SOCKETHELPER_H

class EventLoop;
class TcpConnection;

namespace SocketHelper {

bool InitListenSocket(EventLoop* pEventLoop);
void AcceptConn(EventLoop* pEventLoop, TcpConnection* pListenConn);
void ClientReadCallBack(EventLoop* pEventLoop, TcpConnection* pClientConn);
void CloseSocket(int sockFd);

};



#endif //NET_SOCKETHELPER_H