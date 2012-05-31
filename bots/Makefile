CC=gcc
CFLAGS=-g -Wall

all: CallBot AllIn Probabilistic Conservative Random

bcb-client.o: bcb-client.c bcb-client.h

CallBot.o: CallBot.c bcb-client.h
CallBot: bcb-client.o CallBot.o
	${CC} ${CFLAGS} -o $@ $^

AllIn.o: AllIn.c bcb-client.h
AllIn: bcb-client.o AllIn.o
	${CC} ${CFLAGS} -o $@ $^

Probabilistic.o: Probabilistic.c bcb-client.h
Probabilistic: bcb-client.o Probabilistic.o
	${CC} ${CFLAGS} -o $@ $^

Conservative.o: Conservative.c bcb-client.h
Conservative: bcb-client.o Conservative.o
	${CC} ${CFLAGS} -o $@ $^

Random.o: Random.c bcb-client.h
Random: bcb-client.o Random.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f *.o

fresh: clean all

