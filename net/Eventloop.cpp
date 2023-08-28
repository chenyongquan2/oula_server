#include "Eventloop.h"
#include "Channel.h"
#include "SocketHelper.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <error.h>
#include "Poller.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include "utils/ThreadHelper.h"

const int kPollTimeMs = 10000;

static int createEventFd()
{
    //eventfd是Linux系统提供的一种用于进程间通信的机制，它基于文件描述符，可以用于多个进程之间传递简单的事件通知
    //ntfd返回一个可读写的文件描述符，可以通过read和write函数进行操作。
    // 线程安全：多个线程可以同时操作同一个eventfd，而无需额外的同步措施。
    // 高效通知：当一个进程向eventfd写入一个64位的无符号整数时，其他等待该eventfd的进程可以立即被唤醒。
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd<0)
    {

    }
    return evtfd;
}

EventLoop::EventLoop()
    :threadId_(CurrentThread::gettid())
    ,quit_(false)
    ,poller_(Poller::NewDefaultPoller(this))
    ,wakeupFd_(createEventFd())
    ,isCallingPendingFunctors_(false)
    
{
    wakeupChannel_ = std::make_unique<Channel>(this, wakeupFd_);
    wakeupChannel_->SetReadCallback(
        std::bind(&EventLoop::handleWakeupChannelRaad, this)
    );
    wakeupChannel_->EnableReadEvent();

}

EventLoop::~EventLoop()
{
    //Todo:Can we use RAII to help us to do this?
    wakeupChannel_->DisableAllEvent();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
}

void EventLoop::loop()
{
    std::cout<<"EventLoop start"<<std::endl;
    while(!quit_)
    {
        activeChanels_.clear();
        poller_->poll(kPollTimeMs,&activeChanels_);

        for(auto channel:activeChanels_)
        {
            channel->HandleEvent();
        }

        handlePendingFunctors();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lockGuard(mtx_);
        pendingFunctors_.push_back(cb);
    }

    //有以下两种情况需要唤醒
    //1.当前不是在io线程;2.正在消费pendingFunctors队列，此时来了新任务。
    if(!isInLoopThread() || isCallingPendingFunctors_)
    {
        WakeupToHandlePendingFunctors();
    }

}

void EventLoop::assertInLoopThread()
{
    if(!isInLoopThread())
    {
        //Todo:
        exit(-1);
    }
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == CurrentThread::gettid();
}

void EventLoop::updateChannel(Channel *channel)
{
    //转调poller，因为poller是来监听channel的事件。
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel *channel)
{
    return poller_->HasChannel(channel);
}

void EventLoop::WakeupToHandlePendingFunctors()
{
    //write anything to the wakeupFd_ in order to wakeup the wakeupChannel_
    int one = 1;
    size_t n = ::write(wakeupFd_, (void *)&one, sizeof(one));
}

void EventLoop::handleWakeupChannelRaad()
{
    //这里其实没太大作用，本意是通过此wakeupSocket来唤醒，eventloop去消费pendingFunctors
    int one = 1;
    size_t n = ::read(wakeupFd_, (void *)&one, sizeof(one));
}

void EventLoop::handlePendingFunctors()
{
    isCallingPendingFunctors_ = true;
    std::vector<Functor> pendingFunctors;
    {
        std::lock_guard<std::mutex> lockGuard(mtx_);
        //swap方法是std::vector类的成员函数，用于交换两个vector对象的内容。
        //swap方法是一个高效的操作，它只交换两个vector对象的内部指针， 而不需要复制或移动实际的元素.
        //因此，使用swap方法可以快速地交换两个vector对象，而不会产生额外的开销。
        pendingFunctors.swap(pendingFunctors_);
    }
    
    for(auto func: pendingFunctors)
    {
        func();
    }
    isCallingPendingFunctors_ = false;
}


