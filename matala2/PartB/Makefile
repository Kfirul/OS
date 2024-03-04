CC = gcc
CFLAGS = -lcrypto -lm

all: server asynClient

server:
	$(CC) -o server server.c $(CFLAGS)

asynClient:
	$(CC) -o asynClient asynClient.c $(CFLAGS)

clean:
	rm -f server asynClient
