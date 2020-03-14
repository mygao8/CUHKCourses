CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: cert

cert: cert.cc
	$(CC) $(CFLAGS) cert.cc -o $@ $(LIBS)

clean:
	@rm -f cert
