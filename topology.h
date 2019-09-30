#ifndef TOPOLOGY_H

#define MAX_ROUTERS 20
#define ROUTERS_FILE "roteador.config"
#define LINKS_FILE "enlaces.config"
#define IP_STR_LEN 20

struct router {
  int id;
  int port;
  char ip[IP_STR_LEN];
};

/**
 * matriz de adjacência da rede
 * se o valor for nao negativo, representa o peso da aresta
 * se for -1, nao existe enlace
 */

void set_routers(struct router routers[MAX_ROUTERS]);
void set_network(int network[MAX_ROUTERS][MAX_ROUTERS]);
void set_paths(int network[MAX_ROUTERS][MAX_ROUTERS], int beg, int next_vertex_to[MAX_ROUTERS]);

#define TOPOLOGY_H
#endif
