all: main child

main: main.c
	gcc -Wall -o main.run main.c

child: child.c
	gcc -Wall -o child.run child.c

clean:
	rm -f *.run