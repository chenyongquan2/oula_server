 #include "CurrentThread.h"

//Todo:namespace要使用的变量得定义在namespace里面。
namespace CurrentThread {
    thread_local std::thread::id t_cacheTid;
};
