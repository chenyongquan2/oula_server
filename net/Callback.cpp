#include "Callback.h"
#include "Buffer.h"
#include <string.h>
#include <sys/socket.h>
#include "iostream"
#include "TcpConnection.h"


void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer)
{
    std::string str = buffer->retrieveAllAsString();
    std::cout << str << std::endl;

    //echo to peer
    conn->send(str);
}