#include <stdio.h>
#include <stdlib.h>
#include "shmem_server.h"
int main()
{
    shemem_server_init();
    atexit(shmem_server_close);
    shemem_server_memu();
    return 0;
}
