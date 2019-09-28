#ifndef MESSAGE_H

#define QUEUE_MAX 100

struct string_queue {
  char *_queue[QUEUE_MAX];
  int start, size;
};
void queue_init(struct string_queue *q);
int queue_push(struct string_queue *q, char *message);
char *queue_pop(struct string_queue *q);

#define MESSAGE_H
#endif
