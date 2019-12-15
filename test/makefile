CC = gcc
CFLAGS = -std=c99 -Wall -g -rdynamic -Werror 
LIBS = -lm -pthread


thpool:	
	$(CC) -o thpool $(CFLAGS) ../server/thpool.c ../server/thpool.h test_thpool.c $(LIBS)
