#ifndef MYMYSQL_H_
#define MYMYSQL_H_
#include <mysql/mysql.h>
MYSQL mysql, *conn;
int sql_connect(MYSQL *mysql, MYSQL **conn);
int sql_disconnect(MYSQL *conn);
int query_result(MYSQL *mysql, MYSQL *conn, const char *name, char **buf);

#endif /* MYORACLE_H_ */
