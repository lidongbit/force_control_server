#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/shm.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h> //mmap head file
#include "network_server.h"
#include "force_interface.h"

#define IPSIZE 1024
#define BUFSIZE 1024
#define SERVERPORT "8111"

#define  SERVER_BUFF_SIZE 2048

static int sfd;
static int newsd;
static void network_server_do_memu(int choose);

void network_server_init()
{

    struct sockaddr_in laddr;//local addr
    struct sockaddr_in raddr;//remote addr
    char ip[IPSIZE];

    sfd = socket(AF_INET,SOCK_STREAM,0/*IPPROTO_TCP*/);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }

    int val = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(sfd,1024) < 0){//全连接数量
        perror("listen()");
        exit(1);
    }


    socklen_t raddr_len = sizeof(raddr);

    newsd = accept(sfd,(void *)&raddr,&raddr_len);//接收客户端连接
    if (newsd < 0){
        perror("accept()");
        exit(1);
    }
    inet_ntop(AF_INET,&raddr.sin_addr,ip,IPSIZE);
    printf("client %s %d\n",ip,ntohs(raddr.sin_port));

}

void network_server_close()
{
    close(sfd);
}
static int line_count = 0;
void network_server_memu(void)
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
        char str[10];
        printf("please choose[1~15]:");
        fflush(stdin);
        fgets(str,10,stdin);
        printf("input str: %s\n",str);
        choose = atoi(str);
        printf("atoi: %d\n",choose);
        if(choose<=15 && choose>0)
        {
            network_server_do_memu(choose);
        }
    }
}

static void network_server_do_memu(int choose)
{
    char buff[SERVER_BUFF_SIZE] = {0};
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
    int *app_ptr = (int*)buff;
    printf("ServoCoreProcessCall_t:%d %d %d %d %d %d %d %d\n",*(app_ptr),*(app_ptr+1),*(app_ptr+2),*(app_ptr+3),
                                                                *(app_ptr+4),*(app_ptr+5),*(app_ptr+6),*(app_ptr+7));
    //push_circle_buff_item(local_buff_info,local_buff,buff);
    if (send(newsd,buff,SERVER_BUFF_SIZE,0) < 0)
    {
            perror("send()");
            exit(1);
    }
}
