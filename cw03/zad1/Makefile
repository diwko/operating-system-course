all: main test

main: main.c
	gcc -Wall -o main.run main.c

test: test.c
	gcc -Wall -o test.run test.c

clean:
	rm -f *.run