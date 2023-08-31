
#include "Timestamp.h"

#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <inttypes.h>
#include <sys/time.h>

std::string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

Timestamp Timestamp::getNow()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int seconds = tv.tv_sec;
    //convert to micro seconds.
    return Timestamp(seconds*kMicroSecondsPerSecond + tv.tv_usec);
}