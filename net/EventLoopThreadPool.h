#ifndef NET_EVENTLOOPTHREADPOOL_H
#define NET_EVENTLOOPTHREADPOOL_H



#include "utils/nocopyable.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "EventLoopThread.h"
#include "Eventloop.h"

class EventLoopThreadPool :nocopyable
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
public:
    EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback()); //ThreadInitCallback() is an default empty impl method.
    
    //round-robin to get next loop.
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

private:
    EventLoop * baseloop_;
    std::string name_;
    bool bStartd_;
    int numThreads_;
    int nextLoopIdx_;
    std::vector<std::unique_ptr<EventLoopThread>> thread_;
    std::vector<EventLoop*> loops_;//included baseloop_ at least, it will included other eventloop for io when connection estabished.
};

#endif // !NET_EVENTLOOPTHREADPOOL_H