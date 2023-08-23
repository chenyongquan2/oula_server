#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include <stdint.h>
#include <functional>

//channel的主要作用是根据poll返回的相应消息类型去调用对应callback

class EventLoop;
class Channel
{
public:
    typedef std::function<void()> EventCallback;
public:
    Channel(EventLoop* pEventLoop, int fd);
	~Channel();

    int GetSocketFd();

    //去派发事件，根据不同的event去执行对应类型的callback
    void HandleEvent();
    void SetReadCallback(const EventCallback& cb)
    {
        readCallback_ = cb;
    }
    void SetWirteCallback(const EventCallback& cb)
    {
        writeCallback_ = cb;
    }
    void SetCloseCallback(const EventCallback& cb)
    {
        closeCallback_ = cb;
    }

    int GetAllEvents();
    
    void DisableAllEvent();
    bool IsNoneEvent();

    void EnableReadEvent();
    void DisableReadEvent();
    bool IsEnableReadEvent();

    void EnableWriteEvent();
    void DisableWriteEvent();
    bool IsEnableWriteEvent();
    
    void SetReceiveEvent(int revt);

    //remove the channel's event in the event loop
    void remove();
private:
    //update event in the event loop
    void update();
    
private:
    EventLoop* eventloop_;
    int sockFd_;
    int rEvents_;
    int events_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;


private:
    const int m_socketFd;
    uint32_t m_curEvents;//监听的事件
    uint32_t m_recvEnents;//接受到的事件

};

#endif //NET_CHANNEL_H