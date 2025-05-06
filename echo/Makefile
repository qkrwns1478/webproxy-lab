CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echoserver echoclient

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

echoserver: echoserveri.o echo.o csapp.o
	$(CC) $(CFLAGS) echoserveri.o echo.o csapp.o -o echoserver $(LDFLAGS)

echoclient: echoclient.o echo.o csapp.o
	$(CC) $(CFLAGS) echoclient.o echo.o csapp.o -o echoclient $(LDFLAGS)

echo.o: echo.c csapp.h
	$(CC) $(CFLAGS) -c echo.c

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echoserveri.o: echoserveri.c csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c

clean:
	rm -f *.o echoclient echoserver *~