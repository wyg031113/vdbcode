all:vdb_client vdb_server
CC=gcc
CFLAGS=-O3
LIBS=-lpbc -lgmp
vdb_client_c=vdb.c vdb_client.c
vdb_server_c=vdb.c vdb_server.c
#vdb:vdb.c
#	$(CC) $(CFLAGS) $< $(LIBS) -o $@
vdb_server:$(vdb_server_c)
	$(CC) $(CFLAGS) $(vdb_server_c) $(LIBS) -o $@

vdb_client:$(vdb_client_c)
	$(CC) $(CFLAGS) $(vdb_client_c) $(LIBS) -o $@
clean:
	rm -rf *.o vdb vdb_server vdb_client
