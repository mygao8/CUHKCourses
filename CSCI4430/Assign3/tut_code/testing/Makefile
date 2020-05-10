CC = gcc
LIB =

all: server client

server: server.c
	${CC} -o server server.c ${LIB}

client: client.c
	${CC} -o client client.c ${LIB}

clean:
	rm server
	rm client	
