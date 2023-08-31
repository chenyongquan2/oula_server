#ifndef NET_UTILS_THREADHELPER_H
#define NET_UTILS_THREADHELPER_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>  
#include <thread>  
#include <sstream>  

namespace CurrentThread {

//todo:why it must be inline?
// inline pid_t GetCurTid()
// {
//     //Todo:因为这是一个系统调用很耗时，后面可以引入缓存机制来优化。
//     return static_cast<pid_t>(::syscall(SYS_gettid));
// }

inline std::thread::id GetCurTid()
{
    std::thread::id tid = std::this_thread::get_id();
    return tid;
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


#endif //NET_UTILS_THREADHELPER_H