#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

void queue_init(struct string_queue *q) {
  q->start = q->size = 0;
}

int queue_push(struct string_queue *q, char *message) {
  assert(q->size <= QUEUE_MAX);
  if (q->size == QUEUE_MAX) return 0;

  q->_queue[(q->start + q->size) % QUEUE_MAX] = message;
  q->size++;
  return 1;
}

char *queue_pop(struct string_queue *q) {
  assert(q->size >= 0);
  if (q->size == 0) return NULL;

  char *s = q->_queue[(q->start + q->size) % QUEUE_MAX];
  q->start = (q->start + 1) % QUEUE_MAX;
  q->size--;
  return s;
}
