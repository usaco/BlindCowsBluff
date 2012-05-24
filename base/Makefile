CC=gcc
CFLAGS=-g

all: bcb-curses

bcb-visual.h: bcb-base.h
bcb-curses.o: bcb-curses.c bcb-base.h bcb-visual.h
bcb-opengl.o: bcb-opengl.c bcb-base.h bcb-visual.h

bcb.o: bcb.c bcb-base.h bcb-visual.h

bcb-curses: bcb.o bcb-curses.o
	gcc ${CFLAGS} -o $@ $^ -lcurses

clean:
	rm -f *.o

fresh: clean all

