CXX = g++
CC = gcc
# CPP_FLAGS += -std=c++11
CFLAGS += -shared
CFLAGS += -fPIC

all: preprocess hw2

run: all test
	rm -rf monitor.out
	MONITOR_OUTPUT=monitor.out LD_PRELOAD=./hw2.so ./test
	# MONITOR_OUTPUT=stderr LD_PRELOAD=./hw2.so ./test
	# LD_PRELOAD=./hw2.so ./test
	cat monitor.out

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
	rm -rf monitor.out
