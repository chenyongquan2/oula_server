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

class ThreadPool
{
public:
    ThreadPool(size_t threadNum);
    ~ThreadPool();

    template<typename Func, typename... Args>
    void addTask(Func && f, Args&&... args)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_taskQueue.emplace(
            [=] {
                f(args...);
                //f(std::forward(args...));
            }
        );
        m_cond.notify_one();
    }

private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_bStop;
};

#endif // end THREADPOOL_H