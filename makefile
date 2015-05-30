.SUFFIXES:.c .o 

CC=gcc
SRCS=server.c\
     pub.c\
     work.c\
     mymysql.c

OBJS=$(SRCS:.c=.o)
EXEC=myhttpd

start:$(OBJS)  
	$(CC) -o $(EXEC) $(OBJS) -lpthread -lmysqlclient
	@echo "-------------ok------------"
.c.o:
	$(CC) -Wall -g -o $@ -c $< 
clean:
	rm -f $(OBJS)
	rm -f core*
