all:vdb_test
CC=gcc
CFLAGS=-ggdb3
LIBS=-lpbc -lgmp
vdb_test_c=vdb.c vdb_test.c simple_db.c
#vdb:vdb.c
#	$(CC) $(CFLAGS) $< $(LIBS) -o $@

vdb_test:$(vdb_test_c)
	$(CC) $(CFLAGS) $(vdb_test_c) $(LIBS) -o $@
clean:
	rm -rf *.o vdb vdb_test
