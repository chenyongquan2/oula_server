#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include <stdint.h>

/*
Channel 层一般持有一个 socket（Linux 下也叫 fd） 是实际进行数据收发的地方，
因而一个 Channel 对象会记录当前需要监听的各种网络事件（读写和出错事件）状态，同时提供对这些事件的状态的判断和增删改的接口
Channel 对象管理着 socket 对象的生命周期, 因此Channel 对象也提供对 socket 进行创建和关闭的接口
*/

class EventLoop;
class Channel
{
public:
    Channel(int fd, EventLoop* pEventLoop);
	~Channel();
    int GetSocketFd();
    uint32_t GetCurEvents();
    void SetRecvEvents(uint32_t recvEnents);

    void setEnableReadEvent(bool bEable);
    void setEnableWriteEvent(bool bEable);

private:
    const int m_socketFd;

    EventLoop* m_pEventLoop;
    uint32_t m_curEvents;//监听的事件
    uint32_t m_recvEnents;//接受到的事件

};

#endif //NET_CHANNEL_H