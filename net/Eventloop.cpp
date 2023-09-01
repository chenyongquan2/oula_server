#include "Eventloop.h"
#include "Channel.h"
#include "SocketHelper.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <bits/stdint-uintn.h>
#include <bits/types/sigset_t.h>
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
#include "TimerQueue.h"
#include "utils/CurrentThread.h"
#include "utils/Timestamp.h"
#include <signal.h>
#include <csignal>
#include <unistd.h> 

namespace 
{
    const int kPollTimeMs = 10000;

    
    class IgnoreSigPipe
    {
    public:
        //when the client closed the conn, we still write or send data to the client socket
        //it will throw the SIGPIPE signal, it's default handle function is dump.
        //so we should set the handle function to SIG_IGN to prevent dump happen because the SIGPIPE
        IgnoreSigPipe()
        {
            //写法1
            //::signal(SIGPIPE, SIG_IGN);
            //写法2(好像更推荐？)
            struct sigaction sa;
            sa.sa_handler = &IgnoreSigPipe::signalHandler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags=0;
            if(sigaction(SIGPIPE, &sa, nullptr) == -1)
            {
                std::cout << "IgnoreSigPipe sigaction failed" << std::endl; 
                exit(-1);
            }
            
        }
    private:
        static void signalHandler(int signal) {  
            std::cout << "Received signal: " << signal << std::endl;  
        
            // 执行其他操作，如关闭文件、释放资源等  
        
            // 退出程序  
            //exit(signal);  
        }  
    };

    // 这段代码是用于忽略SIGPIPE信号的处理。SIGPIPE是一个在Unix-like系统中的信号，用于指示一个进程在向一个已经关闭的管道（或者Socket）写数据时发生的错误。默认情况下，当进程向一个已关闭的管道写数据时，系统会向进程发送SIGPIPE信号，导致进程终止。
    IgnoreSigPipe g_ignoreSigPipe;

};

static int createEventFd()
{
    //eventfd是Linux系统提供的一种用于进程间通信的机制，它基于文件描述符，可以用于多个进程之间传递简单的事件通知
    //ntfd返回一个可读写的文件描述符，可以通过read和write函数进行操作。
    // 线程安全：多个线程可以同时操作同一个eventfd，而无需额外的同步措施。
    // 高效通知：当一个进程向eventfd写入一个64位的无符号整数时，其他等待该eventfd的进程可以立即被唤醒。
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    
    // NOTICE：根据Linux的eventfd文档，eventfd是一个用于进程间通信的文件描述符，它的读取和写入操作都使用无符号64位整数。因此，为了正确唤醒eventfd，写入的数据类型必须是uint64_t。
    // 如果您将写入的数据类型改为int，那么编译器可能会发出类型不匹配的警告或错误，并且写入的数据可能无法正确唤醒eventfd。
    // 因此，为了确保正确唤醒eventfd，请使用uint64_t类型的数据进行写入。

    if(evtfd<0)
    {

    }
    return evtfd;
}

int EventLoop::loopNextIdx_ = 0;

EventLoop::EventLoop()
    :threadId_(CurrentThread::tid())
    ,quit_(false)
    ,poller_(Poller::NewDefaultPoller(this))
    ,wakeupFd_(createEventFd())
    ,isCallingPendingFunctors_(false)
    ,timerQueue_(new TimerQueue(this))
{
    loopIdx_ = loopNextIdx_;
    loopNextIdx_++;

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
    //1.当前不是在eventloop所属的线程;2.正在消费pendingFunctors队列，此时来了新任务。
    if(!isInLoopThread() || isCallingPendingFunctors_)
    {
        WakeupToHandlePendingFunctors();
    }

}

void EventLoop::assertInLoopThread()
{
    if(!isInLoopThread())
    {
        exit(-1);
    }
}

bool EventLoop::isInLoopThread() const
{
    auto tid = CurrentThread::tid();
    return threadId_ == tid;
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
    //write 64bits date(must be 64 bits) to the wakeupFd_ in order to wakeup the wakeupChannel_

    //注意：根据Linux的eventfd文档，eventfd是一个用于进程间通信的文件描述符，它的读取和写入操作都使用无符号64位整数。因此，为了正确唤醒eventfd，写入的数据类型必须是uint64_t。
    // 如果您将写入的数据类型改为int，那么编译器可能会发出类型不匹配的警告或错误，并且写入的数据可能无法正确唤醒eventfd。
    // 因此，为了确保正确唤醒eventfd，请使用uint64_t类型的数据进行写入。

    //int one = 1; //write will return -1, error!
    //int32_t one = 1; //write will return -1, error!
    //int64_t one = 1; //write will return -1, error!
    uint64_t one = 1;

    //ssize_t是有符号整数类型，size_t是无符号整数类型。
    ssize_t n = ::write(wakeupFd_, (void *)&one, sizeof(one));
    if(n != sizeof(uint64_t))
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8, error occurs!";
        //todo: throw error
    }
}

void EventLoop::handleWakeupChannelRaad()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, (void *)&one, sizeof(one));
    if(n != sizeof(uint64_t))
    {
        std::cout << "EventLoop::handleWakeupChannelRaad() reads " << n << " bytes instead of 8, error occurs!";
        //todo: throw error
    }
    else
    {
        std::cout << "EventLoop::handleWakeupChannelRaad() sucessed!";
    }
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

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(cb, time, 0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::getNow(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::getNow(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancle(TimerId& timerid)
{
    return timerQueue_->cancle(timerid);
}


