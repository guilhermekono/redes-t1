main: main.c heap.c queue.c
	gcc -o main main.c heap.c queue.c -lpthread -I. -Wall
