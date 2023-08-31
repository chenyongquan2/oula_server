#ifndef NET_UTILS_TIMESTAMP_H
#define NET_UTILS_TIMESTAMP_H

#include <bits/stdint-intn.h>
#include <bits/types/time_t.h>
#include <cstdint> 
#include <string>



class Timestamp
{
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

public:
    Timestamp()
        :microSecondsSinceEpoch_(0)
    {

    }

    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        :microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {
        
    }

    std::string toString() const;
    bool isValid() const 
    {
        return microSecondsSinceEpoch_ > 0;
    }

    int64_t microSecondsSinceEpoch() const
    {
        return microSecondsSinceEpoch_;
    }
    
    time_t secondsSinceEpoch() const
    {
        //time_t是一个表示时间的数据类型，在C和C++中使用。它通常被定义为一个整数类型，用于表示从某个固定时间点（通常是Unix纪元，即1970年1月1日00:00:00 UTC）到现在的秒数。
        return static_cast<time_t>(microSecondsSinceEpoch_/ Timestamp::kMicroSecondsPerSecond);
    }

    //static function.
    static Timestamp getNow();
    static Timestamp getInvalid()
    {
        return Timestamp();
    }

    


private:
    int64_t microSecondsSinceEpoch_;
};

//为啥不写到类的内部呢？
//Todo:override the compare operator
inline bool operator<(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline Timestamp addTime(Timestamp t, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(t.microSecondsSinceEpoch() + delta);
}

#endif //NET_UTILS_TIMESTAMP_H