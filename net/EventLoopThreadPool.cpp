#include "EventLoopThreadPool.h"
#include "Channel.h"
#include "EventLoopThread.h"
#include <cassert>
#include <cstdio>
#include <memory>


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg)
    :baseloop_(baseloop)
    ,name_(nameArg)
    ,bStartd_(false)
    ,numThreads_(0)
    ,nextLoopIdx_(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{
    
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!bStartd_);
    baseloop_->assertInLoopThread();

    bStartd_ = true;

    for(int i=0;i<numThreads_;i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);

        EventLoopThread * t = new EventLoopThread(cb, buf);
        thread_.emplace_back(std::unique_ptr<EventLoopThread>(t));

        //one thread has one loop
        //let main thread(acceptor thread) to wait for sub thread to finish prepare the loop_ and return it.
        loops_.emplace_back(t->startLoop());
    }

    //not sub eventloop/thread
    if(numThreads_ == 0 && cb)
    {
        cb(baseloop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseloop_->assertInLoopThread();
    assert(bStartd_);

    if(loops_.empty())
        return baseloop_;
    
    //round-robin
    EventLoop *loop = loops_[nextLoopIdx_];
    nextLoopIdx_ = (nextLoopIdx_ + 1) % loops_.size();

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    baseloop_->assertInLoopThread();
    assert(bStartd_);
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseloop_);
    }
    else
    {
        return loops_;
    }
}