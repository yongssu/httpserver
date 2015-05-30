#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>



int sql_connect(MYSQL *mysql, MYSQL **conn)
{
    mysql_init(mysql);
    //连接到mysql server
    *conn = mysql_real_connect(mysql, "127.0.0.1", "dbuser1", "sysheng", "db1", 0, 0, 0);
    if(*conn == NULL)
    {
        printf("connect error, %s", mysql_error(mysql));
        return -1;
    }

    if(mysql_query(*conn, "SET NAMES utf8") != 0)
    {
        printf("query error, %s\n", mysql_error(mysql));
    }
    return 1;
}

int sql_disconnect(MYSQL *conn)
{
    mysql_close(conn);
    return 1;
}

//向动态消息体重添加一个url链接
void addurl(const char *url, const char *name, const char *description, char **buf)
{
    char content[1024];
    memset(content, 0, sizeof(content));
    sprintf(content, "<a href=\"http://%s\">%s</a></br>%s%s</br></br>", url, name, name, description);//格式化字符串
    //printf("%s\n",content);

    //addurl函数已经调用过了，所以buf的值不等于NULL
    if(*buf != NULL)
    {
        int buflen = strlen(*buf); //得到buf中字符串的长度
        int contentlen = strlen(content); //得到content中字符串的长度
        int sumlen = buflen + contentlen; //得到buf字符串和content中字符串的长度之和
        char *tmp = malloc(sumlen + 1); //分配一个新的临时缓冲区
        memset(tmp, 0, sumlen + 1);
        strncpy(tmp, *buf, buflen); //将buf中的字符串拷贝到tmp
        strncpy(&tmp[buflen], content, contentlen); //将content中的字符串追加到tmp后面
        free(*buf);
        *buf = tmp; //将buf指向tmp的内存区域
    }
    else //第一次调用addurl函数
    {
        int contentlen = strlen(content); //得到content中字符串的长度
        *buf = malloc(contentlen + 1);//根据content中字符串的长度动态分配内存空间buf
        memset(*buf, 0, contentlen + 1);
        strncpy(*buf, content, contentlen); //将content中字符串拷贝到buf
    }
}

int query_result(MYSQL *mysql, MYSQL *conn, const char *name, char **buf)
{
    char SQL[1024];
    memset(SQL, 0, sizeof(SQL));
    sprintf(SQL, "select * from baidu where name like '%%%s%%'", name);

    if(mysql_query(conn, SQL) != 0)
    {
        printf("query error, %s\n", mysql_error(mysql));
    }
    printf("%s\n", SQL);

    MYSQL_ROW row;
    MYSQL_RES *res;
    res = mysql_store_result(conn);

    if(res == NULL)
    {
        printf("mysql_store_result failed %s\n", mysql_error(mysql));
    }

    int row_num = mysql_num_rows(res);
    if(row_num == 0)
    {
        return -1;
    }
    while(1)
    {
         row = mysql_fetch_row(res);
         if(row == NULL)
         {
             break;
         }

         addurl(row[1], row[2], row[3], buf);
    }
    printf("%s\n", *buf);
    mysql_free_result(res);

    return row_num;
}
