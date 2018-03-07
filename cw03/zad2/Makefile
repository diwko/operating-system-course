all: main test_cpu_time test_memory

main: main.c
	gcc -Wall -o main.run main.c

test_cpu_time: test_cpu_time.c
	gcc -Wall -o test_cpu_time.run test_cpu_time.c

test_memory: test_memory.c
	gcc -Wall -o test_memory.run test_memory.c

clean:
	rm -f *.run