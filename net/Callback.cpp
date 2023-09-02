#include "Callback.h"
#include "Buffer.h"
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "iostream"
#include "TcpConnection.h"
#include "utils/CurrentThread.h"
#include "Eventloop.h"

void TestSigPipe()
{
    sleep(5);
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    std::string str = buffer->retrieveAllAsString();

    auto tid = conn->getLoop()->GetThreadId();
    std::string tidStr = CurrentThread::ConvertThreadId2Str(tid);
    //std::cout << "tid:" << tidStr << ",msg:" << str << std::endl;
    std::cout << "tid:" << tidStr << ",msg:" << std::endl;
    //TestSigPipe();

    //echo to peer
    //for(int i=0;i<1000;i++)
    {
        conn->send(str);
    }
    std::cout << "tid:" << tidStr << "finish to send all str:" << std::endl;
}