
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S), Linux)
    -include Linux.mk
endif
ifeq ($(UNAME_S), Darwin)
    -include Darwin.mk
endif

clean:
	rm -f lc3bsim3-1
	rm -f dumpsim

