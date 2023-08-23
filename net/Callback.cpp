#include "Callback.h"
#include "Buffer.h"
#include <string.h>
#include "iostream"


void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buffer)
{
    std::string str = buffer->retrieveAllAsString();
    std::cout << str << std::endl;
}