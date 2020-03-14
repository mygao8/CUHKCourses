CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: dsa

dsa: dsa.cc
	$(CC) $(CFLAGS) dsa.cc -o $@ $(LIBS)

clean:
	@rm -f dsa
