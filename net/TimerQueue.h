#ifndef NET_TIMEQUQUE_H
#define NET_TIMEQUQUE_H


#include "Channel.h"
#include "Eventloop.h"
#include "Timer.h"
#include "utils/Timestamp.h"
#include "utils/nocopyable.h"
#include <atomic>
#include <utility>
#include <set>
#include <memory.h>

class TimerId;

class TimerQueue :nocopyable
{

private:
    //fix it with std::unique_ptr.
    typedef std::pair<Timestamp, Timer*> Entry;
    //运动Timestamp的排序规则，来在set中自动排序。
    typedef std::set<Entry> TimerList;

    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancle(TimerId timerId);

private:
    void addTimerInLoop(Timer* timer);
    bool insert(Timer* timer);

    void cancelInLoop(TimerId timerId);
    
    //called when timerfd alarms;
    void handelRead();

    //get  expired timers
    std::vector<Entry> getExpired(Timestamp now);
    //reset  expired timers
    void reset(const std::vector<Entry>& expired, Timestamp now);
    
private:
    EventLoop * eventloop_;
    //use timefdChannel_ to observe timefd_'s events
    const int timefd_;
    Channel timefdChannel_;

    //利用set的排序规则。
    TimerList timers_;

    //for cancel
    ActiveTimerSet activeTimers_;//activeTimers_代表还没超时的任务，此时直接取消即可。
    std::atomic<bool> callingExpiredTimers_;//是否正在处理超时任务。
    ActiveTimerSet cancelingTimers_;//需要取消的timer.
    
};


#endif //NET_TIMEQUQUE_H