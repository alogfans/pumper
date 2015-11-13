#include "Status.h"
#include "Types.h"
#include "Socket.h"
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
    char buffer[128] = "Hello world";

    Socket client;
    client.Connect("127.0.0.1", 10086);
    client.SendBytes(buffer, 128);

    memset(buffer, 0, 128);
    client.ReceiveBytes(buffer, 128);
    printf("Received: %s from %s\n", buffer, client.GetAddressPort().c_str());
    client.Close();
    
    return 0;
}
