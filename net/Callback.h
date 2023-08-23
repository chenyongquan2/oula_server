#ifndef NET_CALLBACK_H
#define NET_CALLBACK_H


#include <functional>
#include <memory>

class TcpConnection;
class Buffer;


typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

//新连接accept返回后，触发的回调。
typedef std::function<void(int sockfd)> NewConnectionCallback;
//可读的回调
typedef std::function<void(const TcpConnectionPtr&, Buffer*)> MessageCallback;

//close的回调
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;


void defaultMessageCallback(const TcpConnectionPtr&, Buffer*);


#endif // NET_CALLBACK_H