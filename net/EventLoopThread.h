#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include "Eventloop.h"

class EventLoopThread
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
public:
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop * loop_;
    std::unique_ptr<std::thread> thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

#endif // !NET_EVENTLOOPTHREAD_H