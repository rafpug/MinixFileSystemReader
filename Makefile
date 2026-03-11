CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

all : minls minget

minls : minls.o partition-reader.o
	$(CC) $^ -o $@

minls.o : minls.c
	$(CC) $(CFLAGS) -c $^ -o $@

partition-reader.o : partition-reader.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f minls minls.o partition-reader.o
