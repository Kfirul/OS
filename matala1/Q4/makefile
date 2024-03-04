# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99

# Target executables
TARGETS = myzip myunzip

# Default target
all: $(TARGETS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executables
myzip: myzip.o
	$(CC) $(CFLAGS) myzip.o -o myzip

myunzip: myunzip.o
	$(CC) $(CFLAGS) myunzip.o -o myunzip

# Clean up intermediate object files and executables
clean:
	rm -f *.o $(TARGETS) output.gpg decrypted_output.txt