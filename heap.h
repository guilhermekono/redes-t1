typedef struct vwpair {
  int vertex;
  int weight; // peso para chegar até o vértice
} vwpair;

struct heap {
  vwpair *tree;
  int size;
  int maxsize;
};

void heap_reset(struct heap *heap);

void heap_insert(struct heap *heap, const vwpair p);

vwpair heap_pop(struct heap *heap);
