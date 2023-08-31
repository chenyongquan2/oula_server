#include "TimerQueue.h"
#include "Timer.h"
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <bits/types/struct_timespec.h>
#include <cstddef>
#include <ctime>
#include <functional>
#include <iterator>
#include <sys/timerfd.h>
#include <unistd.h>
#include "TimerId.h"
#include "utils/Timestamp.h"
#include <assert.h>
#include <vector>

static int createTimerfd()
{
    // timerfd_create是一个Linux系统调用函数，用于创建一个用于定时器事件的文件描述符（timer file descriptor）。它可以用于实现定时器功能，通常与其他I/O多路复用机制（如epoll、select等）一起使用。
    // 使用timerfd_create函数创建的定时器文件描述符可以被用作文件I/O操作，以便在指定的时间间隔内触发事件。该函数返回一个整数值，表示创建的定时器文件描述符。

    //CLOCK_REALTIME：使用系统的实时时钟，即系统的墙上时间。它可以用于相对较长的定时器间隔。
    //CLOCK_MONOTONIC：使用系统的单调时钟，它不会随着系统时间的改变而改变。它适用于需要高精度和稳定性的定时器。
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        
    }
    return timerfd;
    //后面的resetTimerFd()负责设置定时器超时时间。
}

void readTimeFd(int timeFd, Timestamp now)
{
    uint64_t howmany;
    size_t n = ::read(timeFd, &howmany, sizeof(howmany));

    //例如，如果我们设置了1秒钟的定时器间隔，那么在每个1秒的时间点，timerfd就会变为可读，并读取到的值将是1。
    if(n!=sizeof(howmany))
    {
        
    }
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = 
        when.microSecondsSinceEpoch() - Timestamp::getNow().microSecondsSinceEpoch();
    
    if(microseconds < 100)
    {
        microseconds = 100;//at least
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    
    return ts;
}

//重置定时器timefd_的到时时间(以队列中所有定时任务的最早的那一个)
void resetTimerFd(int timeFd, Timestamp expiration)
{
    //设置定时器超时时间
    struct itimerspec newvalue;
    struct itimerspec oldvalue;
    memset(&newvalue, 0, sizeof(newvalue));
    memset(&oldvalue, 0, sizeof(oldvalue));
    newvalue.it_value = howMuchTimeFromNow(expiration);

    // timerfd_settime函数用于设置timerfd的超时时间和间隔。
    // 它允许我们指定定时器的初始超时时间和定时器的间隔时间。

    int ret = ::timerfd_settime(timeFd, 0, &newvalue, &oldvalue);
    if(ret)
    {

    }
}


TimerQueue::TimerQueue(EventLoop* loop)
    :eventloop_(loop)
    ,timefd_(createTimerfd())
    ,timefdChannel_(loop, timefd_)
    ,callingExpiredTimers_(false)
{

    timefdChannel_.SetReadCallback(std::bind(&TimerQueue::handelRead,this));
    timefdChannel_.EnableReadEvent();
}

TimerQueue::~TimerQueue()
{
    timefdChannel_.DisableAllEvent();
    timefdChannel_.remove();
    ::close(timefd_);
    //todo: let to test valgrinf to detach.
    // for(auto& timer:timers_)
    // {
    //     delete timer.second;
    // }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(cb,when,interval);
    eventloop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer)
    );

    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    eventloop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if(earliestChanged)
    {
        resetTimerFd(timefd_, timer->expiration());
    }
}

void TimerQueue::cancle(TimerId timerId)
{
    eventloop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId)
    );
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    eventloop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    ActiveTimer timer(timerId.timer_,timerId.seq_);
    auto it = activeTimers_.find(timer);
    if(it != activeTimers_.end())
    {
        //erase the timer directly.
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n==1);
        delete it->first;
        //activeTimers_代表还没超时的任务，此时直接取消即可。
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimers_)//说明此时任务超时，并且正在处理中。
    {
        //不能直接从timers_先移除，因为handelRead->getExpired->正在处理这个超时任务。
        cancelingTimers_.insert(timer);
        //标记此任务后续无需再添加到timer里面，防止后面reset()重新将其添加进去。
    }
    //其余情况说明是已经触发超时的timer，且当前不在任务处理。交由后面的reset()步骤对timer进行重置。
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handelRead()
{
    //  timerfd会变为可读的时机是在达到或超过设置的定时器事件时。当我们创建一个timerfd时，可以使用timerfd_settime函数设置定时器的初始值和间隔。
//  当定时器的计时器到达或超过设置的时间间隔时，timerfd会变为可读，即可通过读取timerfd的值来获取有关定时器事件的信息。

    //Todo:可以看做当poll唤醒时，会把timerfd_给设置为可读。
    eventloop_->assertInLoopThread();
    Timestamp now(Timestamp::getNow());

    readTimeFd(timefd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();

    for(auto const & it:expired)
    {
        it.second->run();
    }

    callingExpiredTimers_ = false;

    //重置定时器的下次到达时间
    reset(expired, now);
}

//返回队头的最早过期时间是否发生了改变。
bool TimerQueue::insert(Timer* timer)
{
    eventloop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    //表示时间队列(按到期先后顺序排序)的队头元素/最早到期时间 是否发生了变化。
    bool earliestChanged = false;
    Timestamp when = timer->expiration();//到期时间
    TimerList::iterator it = timers_.begin();
    if(it==timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }

    //do insert.
    {
        //inert的插入返回为一个pair
        std::pair<TimerList::iterator,bool> result = 
            timers_.insert(Entry(when, timer));
        assert(result.second);

    }
    
    {
        std::pair<ActiveTimerSet::iterator,bool> result =
            activeTimers_.insert(ActiveTimer(timer,timer->sequence()));
        assert(result.second);
    }
    
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    assert(timers_.size() == activeTimers_.size());
    Entry nowEntry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    //binary search,由于std::set中的元素是按照特定的排序规则进行排序的
    //O(log(n))
    TimerList::iterator end = timers_.lower_bound(nowEntry);
    //没找到或者找到了正确了位置。
    assert(end ==timers_.end() || now < end->first);

    //copy to ans
    std::vector<TimerQueue::Entry> expired;
    std::copy(timers_.begin(),end,std::back_inserter(expired) );
    //remove it
    timers_.erase(timers_.begin(), end);

    //把过期到时的timer给从activeTimers_(表示未到时间)中移除掉
    for(auto& it:expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n==1);
    }
    
    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    //remove or reset the expired timer.
    for(auto const & it:expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        if(it.second->repeat()
            &&cancelingTimers_.find(timer) == cancelingTimers_.end())//cancelingTimers_表示不再需要的timer.
        {
            //restart the timer
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            //free the timer
            delete it.second;
        }
    }

    //update the timefd_'s timeout time.
    Timestamp nextExpire;
    if(!timers_.empty())
    {
        //拿队列元素 即最早的时间。
        nextExpire=timers_.begin()->second->expiration();
    }
    if(nextExpire.isValid())
    {
        resetTimerFd(timefd_, nextExpire);
    }
}
