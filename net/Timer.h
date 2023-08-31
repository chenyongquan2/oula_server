#ifndef NET_TIMER_H
#define NET_TIMER_H

#include "utils/Timestamp.h"
#include "utils/nocopyable.h"
#include "Callback.h"

class Timer: nocopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        :callback_(cb)
        ,expiration_(when)
        ,interval_(interval)
        ,repeat_(interval > 0)
        ,seq_(seqTotal++)
    {

    }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return seq_; }

    void restart(Timestamp t);

private:
    TimerCallback callback_;
    Timestamp expiration_;//过期时间
    const double interval_;//interval为间隔的意思
    const bool repeat_;
    const int64_t seq_;
    static int64_t seqTotal;
};

//init 


#endif //NET_TIMER_H