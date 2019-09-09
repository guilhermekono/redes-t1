#include <stdio.h>
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#define MAX_ROUTERS 20
#define ROUTERS_FILE "roteador.config"
#define LINKS_FILE "enlaces.config"

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

int main(void) {
  set_routers();
  printf("\n");
  set_network();
  return 0;
}
