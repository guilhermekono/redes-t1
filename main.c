#include <stdio.h>
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "queue.h"

#define MAX_ROUTERS 20
#define ROUTERS_FILE "roteador.config"
#define LINKS_FILE "enlaces.config"

pthread_mutex_t send_lock, receive_lock, output_lock;
struct string_queue send_queue, receive_queue;
int my_id;

struct router {
  int id;
  int port;
  char *ip;
};

/**
 * matriz de adjacência da rede
 * se o valor for nao negativo, representa o peso da aresta
 * se for -1, nao existe enlace
 */
int network[MAX_ROUTERS][MAX_ROUTERS];

struct router routers[MAX_ROUTERS];

void set_routers(void) {
  int id, port;
  char *ip;
  FILE *file = fopen(ROUTERS_FILE, "r");

  assert(file != NULL);

  while (ip = (char*)malloc(sizeof(char) * 20), 
      fscanf(file, "%d %d %s", &id, &port, ip) == 3) {
    printf("I got: %d %d %s\n", id, port, ip);

    routers[id].id = id;
    routers[id].port = port;
    routers[id].ip = ip;
  }

  free(ip);
  fclose(file);
}

void set_network(void) {
  int id1, id2, weight;
  FILE *file = fopen(LINKS_FILE, "r");

  assert(file != NULL);

  memset(network, -1, MAX_ROUTERS * MAX_ROUTERS);
  while (fscanf(file, "%d %d %d", &id1, &id2, &weight) == 3) {
    assert(id1 < MAX_ROUTERS);
    assert(id2 < MAX_ROUTERS);
    assert(weight >= 0);
    network[id1][id2] = network[id2][id1] = weight;
    printf("I got %d %d %d\n", id1, id2, weight);
  }
}

void *receiver_thread(void *a) {
  while (1) {
    printf("I am the sender thread\n");
  }
}

void *sender_thread(void *a) {
  while (1) {
    printf("I am the receiver thread\n");
  }
}

int main(int argc, char **argv) {
  char msg_content[100];
  int destination;
  pthread_t sender_tid, receiver_tid;

  if (argc < 2) {
    printf("%s\n", argv[0]);
    printf("Missing argument\n");
    printf("Correct usage: %s <router id>\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);
  set_routers();
  printf("\n");
  set_network();

  pthread_create(&sender_tid, NULL, sender_thread, NULL);
  pthread_create(&receiver_tid, NULL, receiver_thread, NULL);

  while (1) {
    printf("Message content: ");
    scanf("%s", msg_content);
    printf("Destination (router id): ");
    scanf("%d", &destination);
  }
  return 0;
}
