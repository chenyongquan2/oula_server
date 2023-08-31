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

//把数据都从发送缓冲区发送到网络协议栈的回调
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;

//close的回调
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;


void defaultMessageCallback(const TcpConnectionPtr&, Buffer*);

//timer的回调
typedef std::function<void()> TimerCallback;


#endif // NET_CALLBACK_H