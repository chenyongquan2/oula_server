#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "net/csocket.h"
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
    ThreadPool pool(4);  
  
    for (int i = 0; i < 8; ++i) {  
        pool.addTask(printHello, i);  
    }  
    for (int i = 0; i < 8; ++i) {  
        pool.addTask(func2, i, i*2);  
    }
  
    // 等待所有任务完成  
    std::this_thread::sleep_for(std::chrono::seconds(1));  
  
    return 0;  
}  

int main()
{
    testThreadpool();
    //Socket mySocket;
    //mySocket.run();

    return 0;
}