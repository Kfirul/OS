CC = gcc
CFLAGS = -Wall -g

all: libencriptor.so encode decode

libencriptor.o: libraryCodec.c libraryCodec.h
	$(CC) $(CFLAGS) -c -fPIC libraryCodec.c -o libencriptor.o

libencriptor.so: libencriptor.o
	$(CC) -shared -o libencriptor.so libencriptor.o

mainEncode.o: encode.c libraryCodec.h
	$(CC) $(CFLAGS) -c encode.c

decode.o: decode.c libraryCodec.h
	$(CC) $(CFLAGS) -c decode.c

encode: libencriptor.so encode.o
	$(CC) $(CFLAGS) -o encode encode.o -L. -lencriptor

decode: libencriptor.so decode.o
	$(CC) $(CFLAGS) -o decode decode.o -L. -lencriptor

clean:
	rm -f libencriptor.o encode.o decode.o encode decode libencriptor.so
#Before running the program use this:export LD_LIBRARY_PATH=.