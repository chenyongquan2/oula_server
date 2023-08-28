#ifndef NET_UTILS_THREADHELPER_H
#define NET_UTILS_THREADHELPER_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {

pid_t gettid()
{
    //Todo:因为这是一个系统调用很耗时，后面可以引入缓存机制来优化。
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

};


#endif //NET_UTILS_THREADHELPER_H