all:vdb
CC=gcc
CFLAGS=-ggdb3
LIBS=-lpbc -lgmp
vdb:vdb.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
clean:
	rm -rf *.o pbc
