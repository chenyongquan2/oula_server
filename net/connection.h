#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H

#include <memory>
#include <iostream>  
#include <tuple>  
#include <utility>
#include <functional>

class Socket;
class Connection;
typedef void(Socket::*EventCallbackType)(Connection*);

class Connection
{
public:
    Connection(int socketFd)
        :m_socketFd(socketFd)
    {
    }
    virtual ~Connection(){}
    int GetSockFd();

public:
    EventCallbackType m_readEventCallbackFunc;//读事件处理回调
    EventCallbackType m_writeEventCallbackFunc;//写事件处理回调

private:
    int m_socketFd;
    /*
    template <typename Callable, typename... Args>
    void setReadEventCallback(Callable&& callable, Args&&... args)
    {
        // m_readCallback = new impl<Callable, Args...>
        //     (std::forward<Callable>(callable), std::forward<Args>(args)...);  
        m_readCallback = std::make_unique< impl<Callable, Args...> >
            (std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template <typename Callable, typename... Args>
    void setWriteEventCallback(Callable&& callable, Args&&... args)
    {
        // m_writeCallback = new impl<Callable, Args...>
        //     (std::forward<Callable>(callable), std::forward<Args>(args)...);  
        m_writeCallback = std::make_unique< impl<Callable, Args...> >
            (std::forward<Callable>(callable), std::forward<Args>(args)...);
    }
private:
    //使用类型擦除
    struct impl_base {
        virtual void invoke() = 0;
        virtual ~impl_base() {}  
    };
    // 具体可调用对象类型的实现类  
    template <typename Callable, typename... Args>
    struct impl: impl_base
    {
        Callable callable_;
        std::tuple<Args...> args_;

        impl(Callable&& callable, Args&&... args)
            :callable_(std::forward(callable))
            ,args_(std::forward(args)...)
        {}

        void invoke() override
        {
            // std::invoke 是用来调用可调用对象本身，可以处理不同类型的可调用对象，包括普通函数、成员函数、函数指针和函数对象。
            // std::apply 是用来将参数以元组的形式传递给可调用对象，并执行调用。它主要用于需要将参数以元组方式传递的情况，以简化代码的书写。
            // 可以说，std::apply 是 std::invoke 的一个补充，用于处理需要将参数以元组方式传递的情况。
            // 它们都提供了一种通用的方式来执行不同类型的函数调用，增强了代码的可读性和灵活性。
            
            //std::invoke(callable_, args_);
            std::apply(callable_, args_);
        }
    };

    //读事件处理函数
    std::unique_ptr<impl_base> m_readCallback;
    //写事件处理函数
    std::unique_ptr<impl_base> m_writeCallback;
    */

};

class ListenConnection: public Connection
{
public:
    ListenConnection(int socketFd);
    virtual ~ListenConnection() {}
};

class ClientConnection: public Connection
{
public:
    ClientConnection(int socketFd);
    virtual ~ClientConnection() {}
};

#endif // end NET_CONNECTION_H