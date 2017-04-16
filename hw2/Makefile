CXX = g++
CC = gcc
# CPP_FLAGS += -std=c++11
CFLAGS += -shared
CFLAGS += -fPIC

all: preprocess hw2

run: all test
	LD_PRELOAD=./hw2.so ./test

hw2: hw2.c
	$(CC) $(CFLAGS) hw2.c -o hw2.so -ldl

test: test.c
	$(CC) -g test.c -o test

preprocess: preprocessor.cpp
	$(CXX) preprocessor.cpp -o pp
	./pp hw2.c

clean:
	rm -rf hw2.so
	rm -rf hw2.c
	rm -rf pp
	rm -rf test
