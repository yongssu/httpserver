#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "pub.h"
#include <mysql/mysql.h>
#include "mymysql.h"

int main(int arg, char *args[])
{
    sql_connect(&mysql, &conn);

    //如果没有参数，main函数返回
    if(arg < 2)
    {
        printf("usage:myserver port\n");
        return EXIT_FAILURE;
    }
    //setdaemon();
    //将第一个参数转化为整数
    int iport = atoi(args[1]);
    if(iport == 0)
    {
        printf("port %d is invalid\n", iport);
        return EXIT_FAILURE;
    }
    int st = socket_create(iport);
    socket_accept(st);
    sql_disconnect(conn);
    close(st);
    printf("myhttp is end\n");
    return 0;
}

