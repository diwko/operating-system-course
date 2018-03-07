all: client server

client: commons.o client.o 
	gcc -Wall commons.o client.o  -o client.out -lpthread

server: commons.o server.o 
	gcc -Wall commons.o server.o  -o server.out -lpthread

client.o: client.c
	gcc -Wall -c client.c -o client.o

server.o: server.c
	gcc -Wall -c server.c -o server.o

commons.o:
	gcc -Wall -c commons.c -o commons.o

clean:
	rm -f *.o *.out 