#ifndef NET_UTILS_CURRENTTHREAD_H
#define NET_UTILS_CURRENTTHREAD_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>  
#include <thread>  
#include <sstream>  

namespace CurrentThread {

extern thread_local std::thread::id t_cacheTid;

inline void cacheTid()
{
    t_cacheTid = std::this_thread::get_id();
}

inline std::thread::id tid()
{
    if(CurrentThread::t_cacheTid == std::thread::id())
    {
        cacheTid();
    }
    return CurrentThread::t_cacheTid;
}

inline std::string ConvertThreadId2Str(const std::thread::id tid)
{
    std::stringstream ss;
    ss << tid;
    std::string str =  ss.str();
    return str;
}

inline std::string GetCurThreadIdStr()
{
    std::thread::id tid = std::this_thread::get_id();
    //std::string str = std::to_string(tid);
    std::stringstream ss;
    ss << tid;
    std::string str =  ss.str();
    return str;
}

};


#endif //NET_UTILS_CURRENTTHREAD_H