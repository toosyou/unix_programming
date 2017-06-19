CXX = g++
CFLAGS	= -Wall -g -I/usr/include/ncurses -std=c++11
LDFLAGS	= -lncurses -pthread -std=c++11
PROGS	= othello

all: $(PROGS)

othello: game.cpp othello.cpp
	$(CXX) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
		rm -f *.o *~ $(PROGS)
