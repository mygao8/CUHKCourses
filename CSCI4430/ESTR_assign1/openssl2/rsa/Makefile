CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: rsa rsa_pem 

rsa: rsa.cc
	$(CC) $(CFLAGS) rsa.cc -o $@ $(LIBS)

rsa_pem: rsa_pem.cc
	$(CC) $(CFLAGS) rsa_pem.cc -o $@ $(LIBS)

clean:
	@rm -f rsa
	@rm -f rsa_pem

