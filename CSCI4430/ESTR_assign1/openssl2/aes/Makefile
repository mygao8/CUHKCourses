CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: aes

aes: aes.cc
	$(CC) $(CFLAGS) aes.cc -o $@ $(LIBS)

clean:
	@rm -f aes
