CC=g++
CFLAGS= -Wall -Wno-sign-compare -Wno-write-strings -O3 -I./
LDFLAGS=-lnfnetlink -lnetfilter_queue -lm -lpthread

EXE=nat
OBJ=nat_cgd.o checksum.o

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LDFLAGS) 

clean:
	/bin/rm -f $(OBJ) $(EXE)
