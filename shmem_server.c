#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h> //mmap head file
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "buffer_manager.h"
#include <string.h>
#include <sys/shm.h>
#include "force_interface.h"

#define  SERVER_BUFF_SIZE 1024
#define  MAX_SIZE 4*1024

typedef buffer_info_t FORCE_DEBUG_RINGS_BUFF_STRUCT;
static void *shm_ctrl = NULL;
static void *shm_msg = NULL;
static int shmid_ctrl,shmid_msg;
static FORCE_DEBUG_RINGS_BUFF_STRUCT *local_buff_info;
static char *local_buff;
static void shmem_server_do_memu(int choose);
void shemem_server_init()
{
    //创建共享内存
    shmid_ctrl = shmget((key_t)1234, sizeof(FORCE_DEBUG_RINGS_BUFF_STRUCT), 0666|IPC_CREAT);
    shmid_msg = shmget((key_t)2345, MAX_SIZE, 0666|IPC_CREAT);
    if( shmid_msg == -1 || shmid_ctrl == -1)
    {
       fprintf(stderr, "shmget failed\n");
       exit(EXIT_FAILURE);
    }

    //将共享内存连接到当前进程的地址空间
    shm_ctrl = shmat(shmid_ctrl, (void*)0, 0);
    shm_msg = shmat(shmid_msg,(void*)0,0);
    if(shm_ctrl == (void*)-1 || shm_msg == (void*)-1)
    {
       fprintf(stderr, "shmat failed\n");
       exit(EXIT_FAILURE);
    }
    printf("Memory attached at 0x%X,0x%X\n", (int)shm_ctrl,(int)shm_msg);

    //设置共享内存
    local_buff_info = (FORCE_DEBUG_RINGS_BUFF_STRUCT*)shm_ctrl;
    local_buff_info->head_index_offset = 0;
    local_buff_info->tail_index_offset = 0;
    local_buff_info->element_length = SERVER_BUFF_SIZE;
    local_buff_info->buff_length = MAX_SIZE;
    local_buff_info->semaphore = 1;
    local_buff = (char*)shm_msg;
}

void shmem_server_close()
{
    if(shmdt(shm_ctrl) == -1 || shmdt(shm_msg) == -1) // 把共享内存从当前进程中分离
    {
       fprintf(stderr, "shmdt failed\n");
       exit(EXIT_FAILURE);
    }

    // 删除共享内存
    if (shmctl(shmid_ctrl, IPC_RMID, 0) == -1 || shmctl(shmid_msg, IPC_RMID, 0) == -1 )
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
}
static int line_count = 0;
void shemem_server_memu(void)
{
    printf("1.CMD_ENABLE\n");
    printf("2.CMD_WORKING\n");
    printf("3.CMD_IDLE\n");
    printf("4.CMD_ABORT\n");
    printf("5.CMD_CONTINUE\n");
    printf("6.CMD_DISABLE\n");
    printf("7.CMD_RESET_FAULT\n");
    printf("8.CMD_SWITCH_MODE\n");
    printf("9.CMD_DIGNOSE_CONFIG\n");
    printf("10.CMD_SET_PARA\n");
    printf("11.CMD_SET_INS\n");
    printf("12.CMD_GET_RES\n");
    printf("13.CMD_SHAKE_REQ\n");
    printf("14.CMD_SHAKE_BIND\n");
    printf("15.CMD_HEART_INC\n");

    while(1)
    {
        int choose = 0;
        int res = 0;
        char str[10];
        printf("please choose[1~15]:");
        fflush(stdin);
        fgets(str,10,stdin);
        printf("input str: %s\n",str);
        choose = atoi(str);
        printf("atoi: %d\n",choose);
        if(choose<=15 && choose>0)
        {
            shmem_server_do_memu(choose);
        }
    }
}

static void shmem_server_do_memu(int choose)
{
    char buff[1024] = {0};
    ServoCoreProcessCall_t app = {0};
    int mode,submode;
    char *ptr = buff+sizeof(ServoCoreProcessCall_t);
    printf("choose:%d\n",choose);
    switch(choose)
    {
    case CMD_ENABLE:
    case CMD_WORKING:
    case CMD_IDLE:
    case CMD_ABORT:  //discard instruction
    case CMD_CONTINUE:
    case CMD_DISABLE:
    case CMD_RESET_FAULT:
        app.cmd = choose;
        break;
    case CMD_SWITCH_MODE:
        app.cmd = choose;
        printf("please input mode and submode:");
        scanf("%d %d",&mode, &submode);
        app.param1 = mode;
        app.param2 =submode;
        break;
    case CMD_DIGNOSE_CONFIG:
        app.cmd = choose;
        *((int*)ptr) = 1;
        *(((int*)ptr)+1) = 1;
        *(((int*)ptr)+2) = 1;
        *(((int*)ptr)+3) = 8;
        break;
    case CMD_SET_PARA:
        app.cmd = choose;
        *((int*)ptr) = 1;
        *(((int*)ptr)+1) = 1;
        *(((int*)ptr)+2) = 1;
        *(((int*)ptr)+3) = 8;
        break;
    case CMD_SET_INS:
        app.cmd = choose;
        *((int*)ptr) = 1;
        *(((int*)ptr)+1) = 1;
        *(((int*)ptr)+2) = 1;
        *(((int*)ptr)+3) = 8;
        break;
    case CMD_SHAKE_REQ:
        app.cmd = choose;
        break;
    case CMD_SHAKE_BIND:
        app.cmd = choose;
        break;
    case CMD_HEART_INC:
        app.cmd = choose;
        break;
    default:
        break;
    }
    memcpy(buff,&app,sizeof(ServoCoreProcessCall_t));
    push_circle_buff_item(local_buff_info,buff,1024);
}
