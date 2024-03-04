CC=gcc
CFLAGS=-pthread -c

# Target for the static library
lib: libproactor.a

# Rule to create the static library
libproactor.a: proactor.o
	ar rcs libproactor.a proactor.o

# Rule to compile proactor.o from proactor.c
proactor.o: proactor.c proactor.h
	$(CC) $(CFLAGS) -o proactor.o proactor.c

# Clean target
.PHONY: clean
clean:
	rm -f *.o *.a
