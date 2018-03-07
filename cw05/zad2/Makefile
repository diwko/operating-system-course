all: main master slave

main: main.c
	gcc -Wall -o main.run main.c

master: master.c
	gcc -Wall -o master.run master.c

slave: slave.c
	gcc -Wall -o slave.run slave.c -lm

clean: 
	rm -f *.run data.txt