zip:
	zip milestone3.zip *.c *.h Makefile

all: main.c sbuffer.c sbuffer.h config.h main.h
	gcc -g *.c -o main.out -lpthread

run: all
	./main.out