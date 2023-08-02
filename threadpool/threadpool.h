#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <utility>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class ThreadPool
{
public:
    ~ThreadPool();
    static ThreadPool* GetInstance();
    
    //这里运用了类型擦除的技巧。
    template<typename Func, typename... Args>
    void addTask(Func && f, Args&&... args)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        //这里对调用的函数封装了一层，使线程task调用可以支持任意类型参数，任意返回值的function形式。
        m_taskQueue.emplace(
            [=]() {
                //f(std::forward<Args>(args)...);
                //f(args...);
                //另外一种写法。
                std::invoke(f,args...);
            }
        );
        m_cond.notify_one();
    }

private:
    //构造函数为私有
    ThreadPool(size_t threadNum);

    //线程入口函数
    void threadFunc();

private:
    //内部类，用于管理ThreadPool此单例类的内存释放
    class ThreadPoolMemGuard
    {
    public:
        ~ThreadPoolMemGuard()
        {
            if(ThreadPool::m_pInstance)
            {
                delete ThreadPool::m_pInstance;
                ThreadPool::m_pInstance = nullptr;
            }
        }
    };

    static ThreadPool* m_pInstance;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_bStop;
};

#endif // end THREADPOOL_H