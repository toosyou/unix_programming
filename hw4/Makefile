CC	= g++
CFLAGS	= -Wall -g -I/usr/include/ncurses -std=c++11
LDFLAGS	= -lncurses -pthread -std=c++11
PROGS	= sample game

all: $(PROGS)

sample: sample.o othello.o
		$(CC) -o $@ $^ $(LDFLAGS)

game: game.o othello.o
		$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
		$(CC) -c $(CFLAGS) $<

%.o: %.cpp
		$(CC) -c $(CFLAGS) $<

clean:
		rm -f *.o *~ $(PROGS)
