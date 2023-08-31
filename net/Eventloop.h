#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include "Callback.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <atomic>
#include "Timer.h"

#include "utils/Timestamp.h"
#include "TimerId.h"
#include <thread>  

class TcpConnection;
class TcpConnectionMgrInterface;

class Poller;
class TimerQueue;

class EventLoop
{
public:
    typedef std::function<void()> Functor;
public:
    EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop) = delete;
    ~EventLoop();

    void loop();
    //if it send to the same loop, it will run within the function, otherwise, it will call queueInLoop
    void runInLoop(Functor cb);
    //make sure the cb will call in whe loop thread, it will run after polling
    void queueInLoop(Functor cb);
    void assertInLoopThread();

    //管理channel相关
    void updateChannel(Channel *);
    void removeChannel(Channel *);
    bool HasChannel(Channel *);
    void WakeupToHandlePendingFunctors();

    //timer's methods
    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancle(TimerId& timerid);

    const std::thread::id GetThreadId() const
        { return threadId_; }

private:
    bool isInLoopThread() const;


    void handleWakeupChannelRaad();
    void handlePendingFunctors();//deal with the pennding tasks.

private:
    const std::thread::id threadId_;
    bool quit_;
    std::unique_ptr<Poller> poller_;

    typedef std::vector<Channel*> ChannelList;
    ChannelList activeChanels_;

    //when the timer timeout, it can wakeup the eventloop to handle read by the wakeChannel
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;//to handel read
    //deal it in the loop
    std::vector<Functor> pendingFunctors_;
    std::mutex mtx_;
    //todo:if it needs really atomic?
    std::atomic<bool> isCallingPendingFunctors_;

};

#endif // end NET_EVENTLOOP_H


