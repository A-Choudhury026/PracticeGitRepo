CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: LRU_Prog

LRU_Prog: main.o LRUCache.o
	$(CC) -o LRU_Prog main.o LRUCache.o

main.o: main.c lrucache.h
	$(CC) $(CFLAGS) -c main.c

LRUCache.o: LRUCache.c LRUCache.h
	$(CC) $(CFLAGS) -c LRUCache.c

clean:
	rm -f *.o LRU_Prog

