#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "net/utils/log.h"

using namespace std;

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000
#define SEND_DATA       "helloworld"

int startClient()
{
    int clientFd=socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1)
    {
        Logger::GetInstance()->error( "create client socket error.");
        return -1;
    }

    sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port=htons(SERVER_PORT);

    int ret=0;
    ret=connect(clientFd, (sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret==-1)
    {
        Logger::GetInstance()->error("connect socket error.");
        return -1;
    }

    ret=send(clientFd, SEND_DATA,strlen(SEND_DATA),0);
    if (ret != strlen(SEND_DATA))
	{
		std::cout << "send data error." << std::endl;
		return -1;
	}

    std::cout << "send data successfully, data: " << SEND_DATA << std::endl;

    char recvBuf[32]={0};
    ret = recv(clientFd,recvBuf,sizeof(recvBuf),0);
    if(ret>0)
    {
        std::cout << "recv data successfully, data: " << recvBuf << std::endl;
    }
    else
    {
        std::cout << "recv data error, data: " << recvBuf << std::endl;
    }

    close(clientFd);

    return 0;
}