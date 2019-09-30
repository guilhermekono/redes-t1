#include "topology.h"
#include "heap.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

void set_routers(struct router routers[MAX_ROUTERS]) {
  int id, port;
  char ip[IP_STR_LEN];
  FILE *file = fopen(ROUTERS_FILE, "r");
  int n_routers = 0;

  assert(file != NULL);

  while (fscanf(file, "%d %d %s", &id, &port, ip) == 3) {
    n_routers++;
    assert(n_routers <= MAX_ROUTERS);
    routers[id].id = id;
    routers[id].port = port;
    strcpy(routers[id].ip, ip);
  }

  fclose(file);
}

void set_network(int network[MAX_ROUTERS][MAX_ROUTERS]) {
  int id1, id2, weight;
  FILE *file = fopen(LINKS_FILE, "r");

  assert(file != NULL);

  memset(network, -1, sizeof(int) * MAX_ROUTERS * MAX_ROUTERS);
  for (int i = 0; i < MAX_ROUTERS; i++) {
    for (int j = 0; j < MAX_ROUTERS; j++) {
      printf("%d ", network[i][j]);
    }
    printf("\n");
  }
  while (fscanf(file, "%d %d %d", &id1, &id2, &weight) == 3) {
    assert(id1 < MAX_ROUTERS);
    assert(id2 < MAX_ROUTERS);
    assert(weight >= 0);
    network[id1][id2] = network[id2][id1] = weight;
  }

  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("-");
  }
  printf("\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    for (int j = 0; j < MAX_ROUTERS; j++) {
      printf("%d ", network[i][j]);
    }
    printf("\n");
  }
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("-");
  }
  printf("\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("%d -> ", i);
    for (int j = 0; j < MAX_ROUTERS; j++) {
      if (network[i][j] != -1) {
        printf(" %d", j);
      }
    }
    printf("\n");
  }
}

static int get_next_vertex_to(int v, 
                              int beg, 
                              int prev_vertex[MAX_ROUTERS]) {
  if (v == -1) return -1;
  if (prev_vertex[v] == beg) return v;
  return get_next_vertex_to(prev_vertex[v], beg, prev_vertex);
}

// needs to be called after set_network
void set_paths(int network[MAX_ROUTERS][MAX_ROUTERS], 
               int beg, 
               int next_vertex_to[MAX_ROUTERS]) {
  int weight[MAX_ROUTERS];
  // weight[i] is the weight of the full path from beg to i
  for (int i = 0; i < MAX_ROUTERS; i++) {
    weight[i] = INT_MAX;
  }

  int prev_vertex[MAX_ROUTERS];
  // prev_vertex[i] is the last vertex visited to get to i
  memset(prev_vertex, -1, sizeof(prev_vertex));

  int ok[MAX_ROUTERS] = {0};

  vwpair vw, pop;
  struct heap heap; // min heap
  heap_init(&heap, MAX_ROUTERS);
  vw.vertex = beg;
  vw.weight = 0;
  weight[beg] = 0;
  prev_vertex[beg] = beg;
  heap_insert(&heap, vw);

  while (heap.size > 0) {
    pop = heap_pop(&heap);
    //weight[pop.vertex] = pop.weight;
    ok[pop.vertex] = 1;

    printf("I popped %d\n", pop.vertex);
    
    for (int i = 0; i < MAX_ROUTERS; i++) {
      vw.vertex = i;
      vw.prev_vertex = pop.vertex;
      vw.weight = weight[pop.vertex] + network[pop.vertex][vw.vertex];

      if (network[beg][i] != -1) {
        printf("this is %d and %d\n", i, network[beg][i]);
      }
      if (network[pop.vertex][i] != -1 && 
          !ok[i] && 
          vw.weight < weight[vw.vertex]) {
        printf("wololo\n");
        weight[vw.vertex] = vw.weight;
        prev_vertex[vw.vertex] = pop.vertex;
        heap_insert(&heap, vw);
      }
    }
  }

  printf("set_paths got to the end\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("%d -> %d\n", prev_vertex[i], i);
  }
  printf("\n");
  for (int i = 0; i < MAX_ROUTERS; i++) {
    printf("%d: %d\n", i, weight[i]);
  }

  for (int i = 0; i < MAX_ROUTERS; i++) {
    next_vertex_to[i] = get_next_vertex_to(i, beg, prev_vertex);
    printf("from %d to %d first go %d\n", beg, i, next_vertex_to[i]);
  }

}
