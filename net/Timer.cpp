#include "Timer.h"
#include "utils/Timestamp.h"

int64_t Timer::seqTotal = 0;

void Timer::restart(Timestamp t)
{
    if(repeat_)
    {
        //下一次到期时间。
        expiration_ = addTime(t, interval_);
    }
    else
    {
        expiration_ = Timestamp::getInvalid();
    }
}