CC=gcc -Wall -g

main: main.c scanner.o utils.o
	$(CC) main.c scanner.o utils.o -o network-diff-scanner

scanner.o: scanner.c scanner.h
	$(CC) -c scanner.c
utils.o: utils.c utils.h
	$(CC) -c utils.c

clean:
	rm *.o

