CC=gcc
CFLAGS=-g -Wall -lncurses

all: human

bcb-client.o: bcb-client.c bcb-client.h

human.o: human.c bcb-client.h
human: bcb-client.o human.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f *.o

fresh: clean all

