#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>  

using namespace std;

int main()
{
    int listenFd=socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        cout << "create listen socket error." << std::endl;
        return -1;
    }

    sockaddr_in bindaddr;
    bindaddr.sin_family=AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(3000);

    int ret=0;
    
    ret=bind(listenFd, (const sockaddr*)&bindaddr, sizeof(bindaddr));
    if(ret==-1)
    {
        std::cout << "bind listen socket error." << std::endl;
        return -1;
    }

    ret=listen(listenFd, SOMAXCONN);
    if(ret==-1)
    {
        std::cout << "listen error." << std::endl;
        return -1;
    }

    while(true)
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if(clientFd !=-1)
        {
            char buf[32]={0};
            int sz=recv(clientFd,buf,sizeof(buf),0);
            if(sz>0)
            {
                std::cout << "recv data from client, data: " << buf << std::endl;
                ret=send(clientFd,buf,sizeof(buf),0);
                if (ret != strlen(buf))
					std::cout << "send data error." << std::endl;
				else
					std::cout << "send data to client successfully, data: " << buf << std::endl;
            
            }
            else 
			{
				std::cout << "recv data error." << std::endl;
			}
        }
        close(clientFd);
    }

    close(listenFd);

    return 0;
}