#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "net/eventloop.h"
#include "threadpool/threadpool.h"

using namespace std;


void printHello(int num) {  
    std::cout << "Hello from thread func1" << num << std::endl;  
}  
int func2(int num1, int num2)
{
    int res=num1+num2;
    std::cout << "Hello from thread func2 " << res << std::endl;  
    return res;
}
  
int testThreadpool() {  
    auto pThreadPool = ThreadPool::GetInstance();
  
    for (int i = 0; i < 8; ++i) {  
        pThreadPool->addTask(printHello, i);  
    }  
    for (int i = 0; i < 8; ++i) {  
        pThreadPool->addTask(func2, i, i*2);  
    }
  
    // 等待所有任务完成  
    std::this_thread::sleep_for(std::chrono::seconds(1));  
    return 0;  
} 

//全局变量和静态变量的初始化。
ThreadPool* ThreadPool::m_pInstance = nullptr;

int main()
{
    //懒汉式启动线程池
    ThreadPool::GetInstance();
    //testThreadpool();
    
    EventLoop eventLoop;
    eventLoop.processEventLoop();

    return 0;
}