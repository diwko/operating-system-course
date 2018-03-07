static: ../zad2/tests.c
	make static -C ../zad1a
	make static -C ../zad1b
	gcc -$(FLAG) -std=c99 -Wall -I../zad1a -I../zad1b -o tests.run ../zad2/tests.c -L../zad1a -L../zad1b -lcontactTree_static -lcontactList_static

shared: ../zad2/tests.c
	make shared -C ../zad1a
	make shared -C ../zad1b
	gcc -$(FLAG) -std=c99 -Wall -I../zad1a -I../zad1b -o tests.run ../zad2/tests.c -L../zad1a -L../zad1b -lcontactTree_shared -lcontactList_shared -Wl,-rpath=../zad1a,-rpath=../zad1b

dynamic: tests_dlfcn.c
	make shared -C ../zad1a
	make shared -C ../zad1b
	gcc -$(FLAG) -std=c99 -Wall -I../zad1a -I../zad1b tests_dlfcn.c -o tests_dlfcn.run -ldl -Wl,-rpath=../zad1a,-rpath=../zad1b
clean:
	rm -f *.so *.a *.o *.run

