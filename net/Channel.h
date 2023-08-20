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
    Channel(int fd, EventLoop* pEventLoop);
	~Channel();

    //去派发事件，根据不同的event去执行对应类型的callback
    void HandleEvent();
    void SetReadCallback(EventCallback callback);
    void SetWirteCallback(EventCallback callback);

    void GetInterestEvent();


    
    void SetReceiveEvent(int revt);
    
private:
    EventLoop* eventloop_;
    int sockFd_;
    int rEvents_;

    EventCallback readCallback_;
    EventCallback writeCallback_;

public:
    int GetSocketFd();
    uint32_t GetCurEvents();
    void SetRecvEvents(uint32_t recvEnents);

    void setEnableReadEvent(bool bEable);
    void setEnableWriteEvent(bool bEable);

private:
    const int m_socketFd;

    
    uint32_t m_curEvents;//监听的事件
    uint32_t m_recvEnents;//接受到的事件

};

#endif //NET_CHANNEL_H