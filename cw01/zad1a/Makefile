static: contact.c contactTree.c
	gcc -c -I. -Wall contact.c contactTree.c
	ar rcs libcontactTree_static.a contact.o contactTree.o
	rm -f *.o

shared: contact.c contactTree.c
	gcc -c -fPIC -I. -Wall contact.c contactTree.c
	gcc -shared -o libcontactTree_shared.so contact.o contactTree.o
	rm -f *.o

clean:
	rm -f *.o *.a *.so
