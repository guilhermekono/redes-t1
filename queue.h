#ifndef MESSAGE_H

#define QUEUE_MAX 2

#define MSG_MSG 0
#define MSG_ACK 1

typedef struct message {
  int type, origin, destination;
  int id; // 0 or 1 (stop and wait)
  char content[101];
} Msg;

struct msg_queue {
  struct message _queue[QUEUE_MAX];
  int start, size;
};

void queue_init(struct msg_queue *q);
int queue_push(struct msg_queue *q, struct message);
int queue_pop(struct msg_queue *q, Msg *msg);

#define MESSAGE_H
#endif
