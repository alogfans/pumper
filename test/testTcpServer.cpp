#include "Status.h"
#include "Types.h"
#include "Socket.h"
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
	char buffer[] = "Hello world";

    TcpServer server;
    server.Listen(10086);

    while (true)
    {
    	TcpClient client;
    	ERROR_ASSERT(server.Accept(client));
    	client.SendBytes(buffer, sizeof(buffer));
    	printf("Handled request from %s\n", client.GetAddressPort().c_str());
    	client.Close();
    }

    server.Close();
    return 0;
}