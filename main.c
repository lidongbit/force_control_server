#include <stdio.h>
#include <stdlib.h>
#include "network_server.h"
int main()
{
    network_server_init();
    atexit(network_server_close);
    network_server_memu();
    return 0;
}
