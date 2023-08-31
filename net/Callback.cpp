#include "Callback.h"
#include "Buffer.h"
#include <string.h>
#include <sys/socket.h>
#include "iostream"
#include "TcpConnection.h"
#include "utils/ThreadHelper.h"
#include "Eventloop.h"


void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    std::string str = buffer->retrieveAllAsString();

    auto tid = conn->getLoop()->GetThreadId();
    std::string tidStr = CurrentThread::ConvertThreadId2Str(tid);
    std::cout << "tid:" << tidStr << ",msg:" << str << std::endl;

    //echo to peer
    conn->send(str);
}