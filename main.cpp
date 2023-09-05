#include <memory>
#include <netinet/in.h>
#include <spdlog/logger.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "net/Eventloop.h"
#include "net/TcpServer.h"
#include "net/utils/InetAddress.h"
#include "threadpool/threadpool.h"
#include "net/utils/log.h"


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

void TestTimer()
{
    static int times = 0;
    std::cout <<"TestTimer call "<< ++times << std::endl;

}

int main()
{
    //懒汉式启动线程池
    //ThreadPool::GetInstance();
    //testThreadpool();
    Logger::GetInstance()->debug("start!");
    
    EventLoop eventLoop;
    InetAddress listenAddr(1234);
    TcpServer tcpServer(&eventLoop, listenAddr);
    tcpServer.setThreadNum(0);
    tcpServer.start();
    
    //test timerqueue.
    //eventLoop.runEvery(1, std::bind(&TestTimer));

    eventLoop.loop();
    

    return 0;
}