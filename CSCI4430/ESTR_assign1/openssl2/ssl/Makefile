CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto -lssl

all: ssl_client ssl_server

ssl_client: ssl_client.c
	$(CC) $(CFLAGS) ssl_client.c -o $@ $(LIBS)

ssl_server: ssl_server.c
	$(CC) $(CFLAGS) ssl_server.c -o $@ $(LIBS)

clean:
	@rm -f ssl_client
	@rm -f ssl_server
