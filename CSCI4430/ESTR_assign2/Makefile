all: myids

myids: myids.c checksum.c
	gcc -Wall myids.c checksum.c -o myids -lpcap -lm

clean:
	@rm myids
