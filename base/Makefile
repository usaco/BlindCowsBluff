CC=gcc
CFLAGS=-g

all: bcb-curses bcb-null

bcb-visual.h: bcb-base.h
bcb-curses.o: bcb-curses.c bcb-base.h bcb-visual.h
bcb-null.o: bcb-null.c bcb-base.h bcb-visual.h
bcb-opengl.o: bcb-opengl.c bcb-base.h bcb-visual.h

bcb.o: bcb.c bcb-base.h bcb-visual.h

bcb-null: bcb.o bcb-null.o
	${CC} ${CFLAGS} -o $@ $^

bcb-curses: bcb.o bcb-curses.o
	${CC} ${CFLAGS} -o $@ $^ -lcurses

clean:
	rm -f *.o

fresh: clean all

