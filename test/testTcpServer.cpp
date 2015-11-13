#include "Status.h"
#include "Types.h"
#include "Socket.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "Singleton.h"
#include "Epoll.h"

#include <stdio.h>

using namespace std;
using namespace Pumper;

std::string read_callback(const TcpConnection& conn, const std::string& msg)
{
    printf("Packet = %s [%s]\n", conn.ToString().c_str(), msg.c_str());
    return msg;
}

int main()
{
/*    
    char buffer[] = "Hello world";

    Socket server;
    server.SetReuseAddress();
    server.Listen(10086);

    while (true)
    {
        Socket client;        
        server.Accept(client);
        client.SendBytes(buffer, sizeof(buffer));
        printf("Handled request from %s\n", client.GetAddressPort().c_str());
        client.Close();
    }

    server.Close();
*/
    TcpServer server;
    server.Start(10086, read_callback);
    Singleton<Epoll>::Instance().Loop();
    return 0;
}
