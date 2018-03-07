all: client server

client: communication_protocol client.c
	gcc -Wall client.c -o client.out -I. -L. -lcommunication_protocol

server: communication_protocol server.c 
	gcc -Wall server.c -o server.out -I. -L. -lcommunication_protocol

communication_protocol: communication_protocol.c
	gcc -c -I. -Wall communication_protocol.c
	ar rcs libcommunication_protocol.a communication_protocol.o
	rm -f *.o

clean: 
	rm -f *.out *.a