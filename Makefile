all:vdb_server vdb_client
CC=gcc
CFLAGS=-ggdb3
LIBS=-lpbc -lgmp -lmysqlclient -lcrypto
vdb_test_c=vdb.c vdb_test.c simple_db.c save_param.c
vdb_server_c=vdb_server.c io.c simple_db.c save_param.c vdb.c
vdb_client_c=vdb_client.c io.c simple_db.c save_param.c vdb.c
#vdb:vdb.c
#	$(CC) $(CFLAGS) $< $(LIBS) -o $@

vdb_test:$(vdb_test_c)
	$(CC) $(CFLAGS) $(vdb_test_c) $(LIBS) -o $@
vdb_server:$(vdb_server_c)
	$(CC) $(CFLAGS) $(vdb_server_c) $(LIBS) -o $@
vdb_client:$(vdb_client_c)
	$(CC) $(CFLAGS) $(vdb_client_c) $(LIBS) -o $@
clean:
	rm -rf *.o vdb vdb_test vdb_server vdb_client param_gen
