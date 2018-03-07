static: contact.c contactList.c
	gcc -c -I. -Wall -std=c99 contact.c contactList.c
	ar rcs libcontactList_static.a contact.o contactList.o
	rm -f *.o

shared: contact.c contactList.c
	gcc -c -fPIC -I. -Wall -std=c99 contact.c contactList.c
	gcc -shared -o libcontactList_shared.so contact.o contactList.o
	rm -f *.o

clean:
	rm -f *.o *.a *.so
