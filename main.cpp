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
#include "net/TcpClient.h"


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

void testClientSendAndRecvData(TcpClient& tcpClient)
{
    //auto connCb = [](const TcpConnectionPtr& conn) -> void
    //如果你不替换 auto，那么 connCb 的类型将被推导为一个 lambda 表达式类型。这个类型的具体形式可能会因编译器而异,会不一定满足ConnectionCallback
    //导致setNewConnectionCallback这一行编译出错。
    ConnectionCallback connCb = [](const TcpConnectionPtr& conn) -> void
    {
        Logger::GetInstance()->debug(
            "[user connectionCallback]local port:{} connect to {}, is:{}",conn->localAddress().toIpPort(), conn->peerAddress().toIpPort(),(conn->isConnected() ? "UP" : "DOWN" )
        );
        //mock send data to server after 3 min
        auto mockSend = [conn]() 
        {
            conn->send("ni hao from client!");
        };

        conn->getLoop()->runAfter(3, std::bind(mockSend));
        
    };

    tcpClient.setNewConnectionCallback(connCb);
}

int main(int argc,char* argv[])
{
    Logger::GetInstance()->debug("argc:{}", argc);
    if(argc <= 1)
    {
        //懒汉式启动线程池
        //ThreadPool::GetInstance();
        //testThreadpool();
        Logger::GetInstance()->debug("server start!");
        
        EventLoop eventLoop;
        InetAddress listenAddr(1234);
        TcpServer tcpServer(&eventLoop, listenAddr);
        tcpServer.setThreadNum(0);
        tcpServer.start();
        
        //test timerqueue.
        //eventLoop.runEvery(1, std::bind(&TestTimer));

        eventLoop.loop();
    }
    else
    {
        Logger::GetInstance()->debug("client start!");
    
        EventLoop eventLoop;
        InetAddress listenAddr(1234);
        TcpClient tcpClient(&eventLoop, listenAddr, "oula_client");

        testClientSendAndRecvData(tcpClient);
        
        tcpClient.enableRetry();//允许与服务端断开链接后重连
        tcpClient.connect();
        
        //test timerqueue.
        //eventLoop.runEvery(1, std::bind(&TestTimer));

        eventLoop.loop();
    }


    return 0;
}