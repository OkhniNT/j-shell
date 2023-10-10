CC=clang
CFLAGS=-O0 -Wall
CLIBS=

all: build run clean

build:
	$(CC) $(CFLAGS) $(CLIBS) main.c -o debug.o

run:
	./debug.o

clean:
	rm debug.o

release:
	$(CC) $(CLIBS) -O2 main.c -o jsh.o
