#ifndef MESSAGE_H

#define QUEUE_MAX 100

typedef struct message {
  char type, origin, destination;
  char content[101];
} Msg;

struct msg_queue {
  struct message _queue[QUEUE_MAX];
  int start, size;
};

void queue_init(struct msg_queue *q);
int queue_push(struct msg_queue *q, struct message);
struct message *queue_pop(struct msg_queue *q);

#define MESSAGE_H
#endif
