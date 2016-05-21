all:vdb
CC=gcc
CFLAGS=-O3
LIBS=-lpbc -lgmp
vdb:vdb.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
clean:
	rm -rf *.o pbc
