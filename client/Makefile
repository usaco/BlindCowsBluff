CC=gcc
CFLAGS=-g

all: call-bot cleverbot

bcb-client.o: bcb-client.c bcb-client.h

call-bot.o: call-bot.c bcb-client.h
call-bot: bcb-client.o call-bot.o
	gcc ${CFLAGS} -o $@ $^

cleverbot.o: cleverbot.c bcb-client.h
cleverbot: bcb-client.o cleverbot.o
	gcc ${CFLAGS} -o $@ $^

clean:
	rm -f *.o

fresh: clean all

