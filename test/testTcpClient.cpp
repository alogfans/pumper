#include "Status.h"
#include "Types.h"
#include "Socket.h"
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
    char buffer[128] = { 0 };

    Socket client;
    client.Connect("127.0.0.1", 10086);
    client.ReceiveBytes(buffer, 128);
    printf("Received: %s from %s\n", buffer, client.GetAddressPort().c_str());
    client.Close();
    
    return 0;
}
