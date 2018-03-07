all: main child main_extra child_extra

main: main.c
	gcc -Wall -o main.run main.c

child: child.c
	gcc -Wall -o child.run child.c

main_extra: main_extra.c
	gcc -Wall -o main_extra.run main_extra.c

child_extra: child_extra.c
	gcc -Wall -o child_extra.run child_extra.c

clean: 
	rm -f *.run