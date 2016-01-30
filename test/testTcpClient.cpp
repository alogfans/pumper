#include "Status.h"
#include "Types.h"
#include "Socket.h"
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
    char buffer[MESSAGE_SIZE] = "Hello world";

    Socket client;
    client.Connect("127.0.0.1", 10086);
    client.SendBytes(buffer, MESSAGE_SIZE);

    memset(buffer, 0, MESSAGE_SIZE);
    client.ReceiveBytes(buffer, MESSAGE_SIZE);
    printf("Received: %s from %s\n", buffer, client.GetAddressPort().c_str());
    client.Close();
    
    return 0;
}
