CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echoserver echoclient

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

echoserver: echoservert.o echo.o csapp.o
	$(CC) $(CFLAGS) echoservert.o echo.o csapp.o -o echoserver $(LDFLAGS)

echoclient: echoclient.o echo.o csapp.o
	$(CC) $(CFLAGS) echoclient.o echo.o csapp.o -o echoclient $(LDFLAGS)

echo.o: echo.c csapp.h
	$(CC) $(CFLAGS) -c echo.c

echoclient.o: echoclient.c csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echoservert.o: echoservert.c csapp.h
	$(CC) $(CFLAGS) -c echoservert.c

clean:
	rm -f *.o echoclient echoserver *~