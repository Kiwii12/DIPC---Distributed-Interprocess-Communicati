#-----------------------------------------------------------------------

CC=g++

CFLAGS=-c -g -Wall -std=c++11

all: DIPC

DIPC: DIPC1
	${CC} -g -Wall -std=c++11 DIPC.o -o DIPC

DIPC1:
	${CC} ${CFLAGS} DIPC.cpp -o DIPC.o

clean:
	rm *o dsh

#-----------------------------------------------------------------------
#g++ -pthread -o DIPC DIPC.cpp -std=c++11