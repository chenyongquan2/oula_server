#ifndef NET_TIMERID_H
#define NET_TIMERID_H

#include "Timer.h"
#include <bits/stdint-intn.h>

class TimerId
{
public:
    TimerId()
        :timer_(nullptr)
        ,seq_(0)
    {}

    TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      seq_(seq)
  {}
    //let TimerQueue can visit cur class's member.
    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t seq_;
};


#endif //NET_TIMERID_H