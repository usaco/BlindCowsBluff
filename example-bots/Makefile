CC=gcc
CFLAGS=-g

all: call-bot

bcb-client.o: bcb-client.c bcb-client.h
call-bot.o: call-bot.c bcb-client.h

call-bot: bcb-client.o call-bot.o
	gcc ${CFLAGS} -o $@ $^

clean:
	rm -f *.o

fresh: clean all

