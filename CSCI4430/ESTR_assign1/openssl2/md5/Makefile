CC=g++
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: md5

md5: md5.cc
	$(CC) $(CFLAGS) md5.cc -o $@ $(LIBS)

clean:
	@rm -f md5
