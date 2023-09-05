#include "Callback.h"
#include "Buffer.h"
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "iostream"
#include "TcpConnection.h"
#include "utils/CurrentThread.h"
#include "Eventloop.h"
#include "utils/log.h"

void TestSigPipe()
{
    sleep(5);
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    std::string str = buffer->retrieveAllAsString();

    auto tid = conn->getLoop()->GetThreadId();
    std::string tidStr = CurrentThread::ConvertThreadId2Str(tid);
    Logger::GetInstance()->debug("tid:{},msg!", tidStr);
    //TestSigPipe();

    //echo to peer
    //for(int i=0;i<1000;i++)
    {
        conn->send(str);
    }
    Logger::GetInstance()->debug("tid:{} finish to send all str", tidStr);
}

void defaultWriteCompleteCallback(const TcpConnectionPtr& conn)
{
    Logger::GetInstance()->debug("conn:{} call defaultWriteCompleteCallback", conn->name());
}

void defaultHighWaterMarkCallback(const TcpConnectionPtr&conn, size_t hithWaterMark)
{
     Logger::GetInstance()->debug("conn:{} call defaultHighWaterMarkCallback, and the hithWaterMark", conn->name());
}