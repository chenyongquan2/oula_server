#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "net/csocket.h"

using namespace std;

int main()
{
    Socket mySocket;
    mySocket.run();

    return 0;
}