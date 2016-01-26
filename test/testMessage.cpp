#include "Status.h"
#include "Types.h"
#include "Message.h"
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Pumper;

int main()
{
    while (1) {
    	char buf[32];
    	scanf("%s", buf);
    	Message test(MessageType::Command, buf);
    	printf("[%s]\n", test.ToPacket().c_str());
    }    
    return 0;
}
