#include "work.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include "mymysql.h"


#define BUFSIZE 8192
#define HEAD "HTTP/1.0 200 OK\n\
Content-Type: %s\n\
Transfer-Encoding: chunked\n\
Connection: Keep-Alive\n\
Accept-Ranges:bytes\n\
Content-Length:%d\n\n"

#define TAIL "\n\n"
#define EXEC "s?wd="


//得到http请求GET后面的文件目录
void gethttpcommand(const char *sHTTPMsg, char *command)
{
    int i;
    int istart = 0;
    int iend = 0;
    for(i = 0; i < strlen(sHTTPMsg); i++)
    {
        if((sHTTPMsg[i]) == ' ' && (istart == 0))
        {
            istart = i + 2;
        }
        else
        {
            if(sHTTPMsg[i] == ' ')
            {
                iend = i;
                break;
            }
        }
    }
    strncpy(command, &sHTTPMsg[istart], (iend - istart));
}



//得到文件内容
int getfilecontent(const char *filename, char **buf)
{
    struct stat t;
    memset(&t, 0, sizeof(t));
    //从只读方式打开参数filename指定的文件
    FILE *fd = fopen(filename, "rb");
    if(fd != NULL)
    {
        stat(filename, &t);
        //根据文件大小，动态分配内存buf
        *buf = malloc(t.st_size);
        //将文件读取到buf
        fread(*buf, t.st_size, 1, fd);
        fclose(fd);
        return t.st_size;
    }
    else
    {
        printf("open %s failed %s\n", filename, strerror(errno));
        return 0;
    }
}

//根据扩展名返回文件类型描述
const char *getfiletype(const char *filename)
{
    //得到文件扩展名
    int len = strlen(filename);
    int i;
    char sExt[32];
    memset(sExt, 0, sizeof(sExt));
    for(i = 0; i < len; i++)
    {
        if(filename[i] == '.')
        {
            strncpy(sExt, &filename[i + 1], sizeof(sExt));
            break;
        }
    }

    //根据扩展名返回相应描述
    if (strncmp(sExt, "bmp", 3) == 0)
        return "image/bmp";

    if (strncmp(sExt, "gif", 3) == 0)
        return "image/gif";

    if (strncmp(sExt, "ico", 3) == 0)
        return "image/x-icon";

    if (strncmp(sExt, "jpg", 3) == 0)
        return "image/jpeg";

    if (strncmp(sExt, "avi", 3) == 0)
        return "video/avi";

    if (strncmp(sExt, "css", 3) == 0)
        return "text/css";

    if (strncmp(sExt, "dll", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "exe", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "dtd", 3) == 0)
        return "text/xml";

    if (strncmp(sExt, "mp3", 3) == 0)
        return "audio/mp3";

    if (strncmp(sExt, "mpg", 3) == 0)
        return "video/mpg";

    if (strncmp(sExt, "png", 3) == 0)
        return "image/png";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/vnd.ms-powerpoint";

    if (strncmp(sExt, "xls", 3) == 0)
        return "application/vnd.ms-excel";

    if (strncmp(sExt, "doc", 3) == 0)
        return "application/msword";

    if (strncmp(sExt, "mp4", 3) == 0)
        return "video/mpeg4";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/x-ppt";

    if (strncmp(sExt, "wma", 3) == 0)
        return "audio/x-ms-wma";

    if (strncmp(sExt, "wmv", 3) == 0)
        return "video/x-ms-wmv";

    return "text/html";
}

//将16进制转化为10进制
int hex2dec(const char hex)
{
    switch(hex)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
        return 10;
    case 'A':
        return 10;
    case 'b':
        return 11;
    case 'B':
        return 11;
    case 'c':
        return 12;
    case 'C':
        return 12;
    case 'd':
        return 13;
    case 'D':
        return 13;
    case 'e':
        return 14;
    case 'E':
        return 14;
    case 'f':
        return 15;
    case 'F':
        return 15;
    default:
        return -1;
    }
}

//将两位16进制的字符串转化为十进制的unsigned char
unsigned char hexstr2dec(const char *hex)
{
    return hex2dec(hex[0]) * 16 + hex2dec(hex[1]);
}


//将HTTP GET请求中的转义符号转化为标准字符，注意空格被转义为+号
void httpstr2stdstr(const char *httpstr, char *stdstr)
{
    int index = 0;
    int i;
    for(i = 0; i < strlen(httpstr); i++)
    {
        if(httpstr[i] == '%')
        {
            stdstr[index] = hexstr2dec(&httpstr[i + 1]);
            i += 2;
        }
        else
        {
            stdstr[index] = httpstr[i];
        }
        index++;
    }
}

//得到模板文件templet.html的内容
int gettempletcontent(char *buf)
{
    struct stat t;
    memset(&t, 0, sizeof(t));

    FILE *fd = fopen("templete.html", "rb");
    if(fd != NULL)
    {
        stat("templete.html", &t);
        fread(buf, t.st_size, 1, fd);
        return t.st_size;
    }
    else
    {
        printf("open %s failed %s\n", "templete.html", strerror(errno));
        return 0;
    }
}

//动态设置http请求内容
int getdynamiccontent(const char *query, char **buf)
{
    char templetecontent[1024];
    memset(templetecontent, 0, sizeof(templetecontent));
    if(gettempletcontent(templetecontent) == 0)
        return 0;

    *buf = malloc(BUFSIZE);
    char *body = NULL;
    if(query_result(&mysql, conn, query, &body) == -1)
    {
        body = malloc(128);
        memset(body, 0, 128);
        strcpy(body, "抱歉，没有查询结果");
    }
    sprintf(*buf, templetecontent, query, body);
    //printf("\nbbbbbbbbbbbb\n%s\n", *buf);
    free(body);
    //sprintf(*buf, templetecontent, query, body);
    return strlen(*buf);
}

//根据get提供的文件名，生成静态http reponse消息内容
int make_http_content(const char *command, char **buf)
{
    char *contentbuf = NULL;
    int icontentlen = 0;

    //GET请求后面为空，得到默认页面内容图
    if(command[0] == 0)
    {
        icontentlen = getfilecontent("default.html", &contentbuf);
    }
    else
    {
        //GET后面请求为s?wd=
        if(strncmp(command, EXEC, strlen(EXEC)) == 0)
        {
            char query[1024];
            memset(query, 0, sizeof(query));
            //得到s?wd=字符串后面的转义字符内容
            httpstr2stdstr(&command[strlen(EXEC)], query);
            icontentlen = getdynamiccontent(query, &contentbuf);
            printf("\naaaaaaaaaaaa\n%s\n", contentbuf);
        }
        else
        {
            icontentlen = getfilecontent(command, &contentbuf);
        }
    }

    if(icontentlen > 0)
    {
        //构造回复消息头
        char headbuf[1024];
        memset(headbuf, 0, sizeof(headbuf));
        sprintf(headbuf, HEAD, getfiletype(command), icontentlen); //设置消息头
        int iheadlen = strlen(headbuf); //得到消息头长度
        int itaillen = strlen(TAIL); //得到消息尾长度
        int isumlen = iheadlen + icontentlen + itaillen; //得到消息总长度
        *buf = malloc(isumlen); //根据消息总长度，动态分配内存
        char *tmp = *buf;
        memcpy(tmp, headbuf, iheadlen); //安装消息头
        memcpy(&tmp[iheadlen], contentbuf, icontentlen); //安装消息体
        memcpy(&tmp[iheadlen + icontentlen], TAIL, itaillen); //安装消息尾

        if(contentbuf)
            free(contentbuf);
        return isumlen;
    }
    else
    {
        return 0;
    }

}

//主要业务函数socket_contr,线程入口函数
void *socket_contr(void *arg)
{
    printf("thread is begin\n");
    int st = *(int *)arg;
    free((int *)arg);

    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    int rc = recv(st, buf, sizeof(buf), 0);

    if(rc <= 0)
    {
        printf("recv failed %s\n", strerror(errno));
    }
    else
    {
        printf("recv is:\n%s\n", buf);
        char command[1024];
        memset(command, 0, sizeof(command));
        //得到http请求中GET后面的字符串
        gethttpcommand(buf, command);
        //printf("%s", command);
        char *content = NULL;
        //根据用户在GET中的请求，生成相应的请求内容
        int ilen = make_http_content(command, &content);

        printf("\n\n%s\n\n", content);
        if(ilen > 0)
        {
            send(st, content, ilen, 0); //将回复的内容发型给client端socket
            free(content);
        }
    }
    close(st); //关闭客户端client
    printf("thread_is end\n");
    return NULL;
}
