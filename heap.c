#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

void heap_init(struct heap *heap, const int maxsize) {
  heap->tree = (vwpair*) malloc(sizeof(vwpair) * maxsize);

  heap->maxsize = maxsize;
  heap->size = 0;
}

void heap_end(struct heap *heap) { free(heap->tree); }

void heap_reset(struct heap *heap) { heap->size = 0; }

static int heap_compare(const vwpair a, const vwpair b) {
  return a.weight > b.weight;
}

static int heap_lchild(const int i) { return 2 * i + 1; }
static int heap_rchild(const int i) { return 2 * i + 2; }
static int heap_parent(const int i) { return (i - 1) / 2; }

static void heap_swap(struct heap *heap, const int i, const int j) {
  vwpair temp;

  temp = heap->tree[i];
  heap->tree[i] = heap->tree[j];
  heap->tree[j] = temp;
}

static void heap_moveup(struct heap *heap, const int i) {
  int pi;

  if (i <= 0) return;

  pi = heap_parent(i);

  if (heap_compare(heap->tree[pi], heap->tree[i])) {
    heap_swap(heap, pi, i);
    heap_moveup(heap, pi);
  }
}

static void heap_movedown(struct heap *heap, const int i) {
  int li, ri;

  if (i >= heap->size) return;

  li = heap_lchild(i);
  ri = heap_rchild(i);

  if (li <= heap->size && heap_compare(heap->tree[i], heap->tree[li])) {
    heap_swap(heap, i, li);
    heap_movedown(heap, li);
    heap_movedown(heap, i);
  } else if (ri <= heap->size && heap_compare(heap->tree[i], heap->tree[ri])) {
    heap_swap(heap, i, ri);
    heap_movedown(heap, ri);
    heap_movedown(heap, i);
  }
}

void heap_insert(struct heap *heap, const vwpair p) {
  heap->size++;
  heap->tree[heap->size - 1] = p;
  heap_moveup(heap, heap->size - 1);
}

vwpair heap_pop(struct heap *heap) {
  vwpair head;
  if (heap->size <= 0) {
    head.vertex = -1;
    head.weight = -1;
    return head;
  }

  head = heap->tree[0];
  heap->tree[0] = heap->tree[heap->size - 1];
  heap->size--;
  heap_movedown(heap, 0);
  return head;
}


void heap_print(const struct heap *heap, const int i) {
  int li, ri, aux;

  if (i >= heap->size || i < 0) return;

  li = heap_lchild(i);
  ri = heap_rchild(i);

  heap_print(heap, li);
  aux = i + 1;
  while (aux >>= 1, aux) {
    printf("    ");
  }
  printf("w:%d, v:%d (%d)\n", heap->tree[i].weight, 
      heap->tree[i].vertex, i);
  heap_print(heap, ri);
}

int main(void) {
  struct heap heap;
  vwpair p;
  heap_init(&heap, 100);
  heap_reset(&heap);
  int op;
  vwpair val;
  while (scanf("%d", &op) != EOF) {
    if (op == 0) {
      p = heap_pop(&heap);
      printf("popped (w:%d, v:%d)\n=======\n", p.weight, p.weight);
      heap_print(&heap, 0);
    } else if (op == 1) {
      printf("v w\n");
      scanf("%d %d", &val.vertex, &val.weight);

      heap_insert(&heap, val);
      heap_print(&heap, 0);
      for (int i = 0; i < heap.size; i++) {
        printf("%d ", heap.tree[i]);
      }
      printf("\n");
    }
    printf("\n+++\n");
  }
  return 0;
}
