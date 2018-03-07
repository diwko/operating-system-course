all: barber client

client: fifo.o client.o
	gcc -Wall fifo.o client.o -o client.run -lrt -lpthread

barber: fifo.o barber.o
	gcc -Wall fifo.o barber.o -o barber.run -lrt -lpthread

client.o: client.c
	gcc -Wall -c client.c -o client.o -I. 

barber.o: barber.c
	gcc -Wall -c barber.c -o barber.o -I. 

fifo.o: fifo.c
	gcc -Wall -c fifo.c -o fifo.o -I.

clean:
	rm -f fifo.o client.o barber.o client.out barber.out