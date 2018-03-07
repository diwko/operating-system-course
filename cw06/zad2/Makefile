all: client server

client: communication_protocol client.c
	gcc -Wall client.c -o client.out -I. -L. -lcommunication_protocol -Wl,-rpath=.

server: communication_protocol server.c 
	gcc -Wall server.c -o server.out -I. -L. -lcommunication_protocol -Wl,-rpath=.

communication_protocol: communication_protocol.c
	gcc -c -fPIC -I. -Wall communication_protocol.c
	gcc -shared -o libcommunication_protocol.so communication_protocol.o -lrt
	rm -f *.o

clean: 
	rm -f *.out *.so