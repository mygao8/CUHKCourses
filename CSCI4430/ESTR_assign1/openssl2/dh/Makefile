CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: dh

dh: dh.cc
	$(CC) $(CFLAGS) dh.cc -o $@ $(LIBS)

clean:
	@rm -f dh

