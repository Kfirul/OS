CC = gcc
CFLAGS = -Wall -Wextra -pthread
INCLUDES = -I partB  

all: partC/proactorServer PartA_build

PartA_build:
	@$(MAKE) -C partA

partC/proactorServer: partB/libproactor.a partC/proactorServer.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -LpartB -lproactor

partB/libproactor.a: partB/proactor.o
	ar rcs $@ $^

partB/proactor.o: partB/proactor.c partB/proactor.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f partC/proactorServer partB/proactor.o partB/libproactor.a
	@$(MAKE) -C partA clean
