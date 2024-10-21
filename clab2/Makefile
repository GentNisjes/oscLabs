all: switch temp debug

switch: switchPointers.c
	gcc -o switch switchPointers.c

temp: tempMeasurement.c
	gcc -o temp tempMeasurement.c

debug: debug.c
	gcc -o debug debug.c -g -W -Wall

clean:
	rm -rf *.o switch temp
