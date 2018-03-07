all: main zad1a zad1b zad1c

main: main.c
	gcc -Wall main.c -o main.run -pthread

zad1a: zad1a.c
	gcc -Wall zad1a.c -o zad1a.run -pthread

zad1b: zad1b.c
	gcc -Wall zad1b.c -o zad1b.run -pthread

zad1c: zad1c.c
	gcc -Wall zad1c.c -o zad1c.run -pthread

clean:
	rm -f main.run zad1a.run zad1b.run zad1c.run