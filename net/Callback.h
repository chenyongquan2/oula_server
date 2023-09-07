#ifndef NET_CALLBACK_H
#define NET_CALLBACK_H


#include <cstddef>
#include <functional>
#include <memory>

class TcpConnection;
class Buffer;

//这里都是user级别的回调。

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

//新连接accept返回后，触发的回调。
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
//可读的回调
typedef std::function<void(const TcpConnectionPtr&, Buffer*)> MessageCallback;

//把数据都从发送缓冲区发送到网络协议栈的回调
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;

//当要发送的数据达到了一个高水平后，执行此回调
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

//close的回调
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;

//default callbacl
void defaultConnectionCallback(const TcpConnectionPtr&);

void defaultMessageCallback(const TcpConnectionPtr&, Buffer*);

void defaultWriteCompleteCallback(const TcpConnectionPtr&);

void defaultHighWaterMarkCallback(const TcpConnectionPtr&, size_t);

//timer的回调
typedef std::function<void()> TimerCallback;


#endif // NET_CALLBACK_H