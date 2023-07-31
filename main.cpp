#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "net/csocket.h"
#include "threadpool/threadpool.h"

using namespace std;


void printHello(int num) {  
    std::cout << "Hello from thread " << num << std::endl;  
}  
  
int testThreadpool() {  
    ThreadPool pool(4);  
  
    for (int i = 0; i < 8; ++i) {  
        pool.addTask(printHello, i);  
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