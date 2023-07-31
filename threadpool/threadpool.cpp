#include "threadpool.h"
#include <atomic>
#include <functional>
#include <mutex>

ThreadPool::ThreadPool(size_t threadNum)
    :m_bStop(false)
{
    for(auto i=0;i<threadNum;++i)
    {
        m_threads.emplace_back(&ThreadPool::threadFunc, this);
    }
}

ThreadPool::~ThreadPool()
{
    m_bStop.store(true, std::memory_order_seq_cst);
    m_cond.notify_all();//通知线程把手头的活给收尾了
    for(auto& t:m_threads)
    {
        if(t.joinable())
        {
            t.join();
        }
    }

}

//线程入口函数
void ThreadPool::threadFunc()
{
    while(true)
    {
        std::function<void()> curTask;
        //减少锁的粒度
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            //while(m_taskQueue.empty() && !m_bStop)
            m_cond.wait(lock, 
                [this] {return m_bStop || !m_taskQueue.empty();}//防止虚假唤醒。
            );
            if(m_bStop.load(std::memory_order_acquire) && m_taskQueue.empty())
            {
                //当m_bStop但是m_taskQueue不为空，应该要把剩余的任务都给消费完，才退出线程。
                return;
            }
            curTask=std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }
        
        //消费任务
        curTask();
        
    }
}

