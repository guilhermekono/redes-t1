#ifndef HEAP_H

#include "queue.h"
#include "topology.h"

typedef struct vwpair {
  int prev_vertex;
  int vertex;
  int weight; // peso para chegar até o vértice
} vwpair;

struct heap {
  vwpair tree[MAX_ROUTERS];
  int size;
  int maxsize;
};

void heap_init(struct heap *heap, const int maxsize);

void heap_reset(struct heap *heap);

void heap_insert(struct heap *heap, const vwpair p);

vwpair heap_pop(struct heap *heap);
#define HEAP
#endif
