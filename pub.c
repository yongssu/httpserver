#include "pub.h"
#include "work.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

void setdaemon()
{
    pid_t pid, sid;
    pid = fork();
    if (pid < 0)
    {
        printf("fork failed %s\n", strerror(errno));
        exit (EXIT_FAILURE);
        ;
    }
    if (pid > 0)
    {
        exit (EXIT_SUCCESS);
    }

    if ((sid = setsid()) < 0)
    {
        printf("setsid failed %s\n", strerror(errno));
        exit (EXIT_FAILURE);
    }

    /*
     if (chdir("/") < 0)
     {
     printf("chdir failed %s\n", strerror(errno));
     exit(EXIT_FAILURE);
     }
     umask(0);
     close(STDIN_FILENO);
     close(STDOUT_FILENO);
     close(STDERR_FILENO);
     */
}

//将struct sockaddr_in转化为IP地址字符串
void sockaddr_toa(const struct sockaddr_in *addr, char *IPAddr)
{
    unsigned char *p = (unsigned char *)&(addr->sin_addr.s_addr);
    sprintf(IPAddr, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
}



//根据参数port，建立server端socket
int socket_create(int port)
{
    int st = socket(AF_INET, SOCK_STREAM, 0);
    if(st == -1)
    {
        printf("socket failed %s\n", strerror(errno));
        return 0;
    }

    //设置端口可重用
    int on = 1;
    if(setsockopt(st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        printf("setsockopt failed %s\n", strerror(errno));
        return 0;
    }

    //设置socket地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(st, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("bind failed %s\n", strerror(errno));
        return 0;
    }
    if(listen(st, 100) == -1)
    {
        printf("listen failed %s\n", strerror(errno));
        return 0;
    }
    printf("listen %d  success\n", port);
    return st;

}


void socket_accept(int st)
{
    pthread_t thr_d;
    pthread_attr_t attr;
    pthread_attr_init(&attr); //初始化pthread的attr
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //设置线程为可分离状态
    int client_st = 0;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    //循环执行accept
    while(1)
    {
        memset(&client_addr, 0, sizeof(client_addr));

        //accept函数阻塞，直到有client端连接到达，或者accept错误返回
        int client_st = accept(st, (struct sockaddr*)&client_addr, &len);

        if(client_st == -1)
        {
            printf("accept failed %s\n", strerror(errno));
            break; //accept错误，循环break
        }
        else
        {
            char sIP[32];
            memset(sIP, 0, sizeof(sIP));
            sockaddr_toa(&client_addr, sIP);
            printf("Accept by %s\n", sIP);

            //创建新线程
            int *tmp = malloc(sizeof(int));
            *tmp = client_st;
            {
                //将来自client端的socket作为参数，启动一个可分离线程
                pthread_create(&thr_d, &attr, socket_contr, tmp);
            }
        }
    }
    pthread_attr_destroy(&attr);
}


























