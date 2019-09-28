#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

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

Msg *queue_pop(struct msg_queue *q) {
  assert(q->size >= 0);
  if (q->size == 0) return NULL;

  Msg msg = q->_queue[(q->start + q->size) % QUEUE_MAX];
  q->start = (q->start + 1) % QUEUE_MAX;
  q->size--;

  Msg *m = (Msg*)malloc(sizeof(Msg));
  *m = msg;

  return m;
}

char *message_to_string(Msg msg) {
  char *str = (char*)malloc(sizeof(char)*104);
  str[0] = msg.type;
  str[1] = msg.origin;
  str[2] = msg.destination;
  strcpy(str + 3, msg.content);

  return str;
}

Msg *string_to_message(const char *str) {
  Msg *msg = (Msg*)malloc(sizeof(Msg));
  msg->type = str[0];
  msg->origin = str[1];
  msg->destination = str[2];
  strcpy(msg->content, str + 3);
  return msg;
}
