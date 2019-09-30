main: main.c heap.c queue.c topology.c
	gcc -o main main.c heap.c queue.c topology.c -lpthread -I. -Wall
