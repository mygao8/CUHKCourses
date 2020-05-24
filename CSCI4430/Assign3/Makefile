CC=g++
CFLAGS= -Wall -Wno-sign-compare -Wno-write-strings -g -O3 -I./
LDFLAGS=-g -lnfnetlink -lnetfilter_queue -lm -lpthread

EXE=nat
OBJ=nat.o checksum.o

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LDFLAGS) 

clean:
	/bin/rm -f $(OBJ) $(EXE)
