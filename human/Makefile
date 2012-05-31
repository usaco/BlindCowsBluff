CC=gcc
CFLAGS=-g -Wall

all: human-curses

bcb-client.o: bcb-client.c bcb-client.h

human-curses.o: human-curses.c bcb-client.h
human-curses: bcb-client.o human-curses.o
	${CC} ${CFLAGS} -o $@ $^ -lncurses

clean:
	rm -f *.o

fresh: clean all

