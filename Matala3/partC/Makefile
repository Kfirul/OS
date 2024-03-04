CC = gcc -g
CFLAGS = -Wall
LDFLAGS = -lpthread

# Path to proactor.h
PROACTOR_INCLUDE = /home/roy/Desktop/OperationS3/partB

all: ProactorServer

# Build the server with proactor
libserver: server_proactor.o libproactor.a
	gcc -o libserver server_proactor.o -L. -lproactor -pthread

server_proactor.o: server_proactor.c
	gcc -c -o server_proactor.o server_proactor.c -I.
	

clean:
	rm -f libserver libproactor.a server_proactor.o


.PHONY: all clean
