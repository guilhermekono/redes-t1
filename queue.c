#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

void queue_init(struct msg_queue *q) {
  q->start = q->size = 0;
}

int queue_push(struct msg_queue *q, Msg msg) {
  assert(q->size <= QUEUE_MAX);
  if (q->size == QUEUE_MAX) return 0;

  q->_queue[(q->start + q->size) % QUEUE_MAX] = msg;
  q->size++;
  return 1;
}

int queue_pop(struct msg_queue *q, Msg *msg) {
  assert(q->size >= 0);
  if (q->size == 0) return 0;
  
  *msg = q->_queue[q->start % QUEUE_MAX];
  q->start = (q->start + 1) % QUEUE_MAX;
  q->size--;
  return 1;
}
