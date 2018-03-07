static: tests.c
	make static -C ../zad1a
	make static -C ../zad1b
	gcc -std=c99 -Wall -g -I../zad1a -I../zad1b -o tests.run tests.c -L../zad1a -L../zad1b -lcontactTree_static -lcontactList_static

dynamic: tests.c
	make shared -C ../zad1a
	make shared -C ../zad1b
	gcc -std=c99 -Wall -g -I../zad1a -I../zad1b -o tests.run tests.c -L../zad1a -L../zad1b -lcontactTree_shared -lcontactList_shared -Wl,-rpath=../zad1a,-rpath=../zad1b

clean:
	rm -f *.so *.a *.o *.run
