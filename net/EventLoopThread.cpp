#include "EventLoopThread.h"
#include <cassert>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    :loop_(nullptr)
    ,callback_(cb)
{
    
}

void EventLoopThread::threadFunc()
{
    EventLoop ioLoop;
    if(callback_)
    {
        callback_(&ioLoop);
    }
    
    {
        //because the main thread will call startLoop, it will race with this sub-thread.
        std::lock_guard<std::mutex> lockGuard(mutex_);
        loop_ = &ioLoop;
        cond_.notify_one();
    }

    ioLoop.loop();//it will block in this pos.
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        loop_ = nullptr;
    }

    //the ioLoop is in the stack mem, it will deallocated atter this scope.
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_);
    //start the thread, and the sub thread will call threadFunc
    thread_ =  std::make_unique<std::thread>(std::bind(&EventLoopThread::threadFunc, this));


    //let main thread(acceptor thread) to wait for sub thread to finish prepare the loop_
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lockGuard(mutex_);
        //while(loop_ == nullptr)//it must be while instead of if, bcz the fake wakeup.
        {
            cond_.wait(lockGuard, [this] {return loop_ != nullptr;});
        }
        loop = loop_;
    }

    return loop;
}

EventLoopThread::~EventLoopThread()
{
    if(loop_)
    {
        //loop_->quirt();
        if(thread_->joinable())
            thread_->join();
    }
}

