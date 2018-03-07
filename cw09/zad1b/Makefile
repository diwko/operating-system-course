main: main.o fifo.o
	gcc -Wall fifo.o main.o -o main.out -lpthread

main.o: main.c
	gcc -Wall -c main.c -o main.o -I.

fifo.o: fifo.c
	gcc -Wall -c fifo.c -o fifo.o -I.

clean:
	rm -f fifo.o main.o main.out