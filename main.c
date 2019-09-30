#include <stdio.h>
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"
#include "topology.h"

pthread_mutex_t send_lock[MAX_ROUTERS], receive_lock, output_lock;
sem_t send_sem[MAX_ROUTERS], receive_sem;
sem_t ack_sem[MAX_ROUTERS];
struct msg_queue send_queue[MAX_ROUTERS], receive_queue;
int my_id, my_socket;
struct router routers[MAX_ROUTERS];
int next_router_to[MAX_ROUTERS];
int network[MAX_ROUTERS][MAX_ROUTERS];

void error(const char *str) {
  fprintf(stderr, "Error: %s\n", str);
  exit(1);
}

void *receiver_thread(void *a) {
  Msg msg;
  struct sockaddr_in my_addr;
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons((uint16_t)routers[my_id].port);
  if (inet_aton(routers[my_id].ip, &my_addr.sin_addr) == 0) {
    error("inet_aton FAILED in receiver_thread");
  }

  if (bind(my_socket, (const struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
    error("bind FAILED in receiver_thread");
  }

  while (1) {
    if (recv(my_socket, (void*)&msg, sizeof(msg), 0) == -1) {
      error("recv FAILED in receiver_thread");
    }

    printf("I got a message\n");
    printf("%s\n", msg.content);

    if (msg.destination != my_id) { // message not for me
      int next_router = next_router_to[msg.destination];
      printf("I am forwarding a message from %d to %d, next router is %d\n", msg.origin, msg.destination, next_router);
      pthread_mutex_lock(&send_lock[next_router]);
      if (queue_push(&send_queue[next_router], msg)) {
        sem_post(&send_sem[next_router]);
      }
      pthread_mutex_unlock(&send_lock[next_router]);
    } else if (msg.type == MSG_ACK && msg.destination == my_id) { // confirmation for me
      printf("I got an ACK\n");
      sem_post(&ack_sem[msg.origin]);
    } else if (msg.type == MSG_ACK && msg.destination != my_id) { // confirmation for another router
      int next_router = next_router_to[msg.destination];
      pthread_mutex_lock(&send_lock[next_router]);
      if (queue_push(&send_queue[next_router], msg)) {
        sem_post(&send_sem[next_router]);
      }
      pthread_mutex_unlock(&send_lock[next_router]);
    } else { // message for me
      printf("THERE IS A MESSAGE FOR ME\n");
      pthread_mutex_lock(&send_lock[msg.destination]);
      int aux = msg.origin; // swap origin and destination
      msg.origin = msg.destination;
      msg.destination = aux;
      msg.type = MSG_ACK;
      if (queue_push(&send_queue[msg.destination], msg)) {
        sem_post(&send_sem[msg.destination]);
      }
      pthread_mutex_unlock(&send_lock[msg.destination]);
    }

    pthread_mutex_lock(&receive_lock);
    queue_push(&receive_queue, msg);
    pthread_mutex_unlock(&receive_lock);
  }
}

void *sender_thread(void *arg) {
  // this thread is responsible to sending
  // messages to the router with id tid
  int tid = *(int*)arg;
  free(arg);
  Msg msg;
  struct sockaddr_in addr_dest;
  addr_dest.sin_family = AF_INET;
  int attempts, confirmed;
  struct timespec time_wait;

  while (1) {
    //printf("I am the receiver thread\n");
    sem_wait(&send_sem[tid]);
    pthread_mutex_lock(&send_lock[tid]);
    queue_pop(&send_queue[tid], &msg);
    pthread_mutex_unlock(&send_lock[tid]);
    printf("I will try to send a message\n");

    addr_dest.sin_port = htons((uint16_t)routers[tid].port);
    printf("the destination is %d\n", msg.destination);
    if (inet_aton(routers[tid].ip, 
          &addr_dest.sin_addr) == -1) {
      error("inet_aton FAILED in sender_thread");
    }

    int reliable_del = msg.type == MSG_MSG && network[my_id][msg.destination] != -1;
    if (reliable_del) {
      printf("I will use reliable delivery\n");
    } else {
      printf("I will NOT use reliable delivery\n");
    }

    attempts = reliable_del ? 3 : 1;
    confirmed = 0;
    while (attempts > 0 && !confirmed) {
      if (sendto(my_socket, 
            &msg, 
            sizeof(msg), 
            0, 
            (struct sockaddr*)&addr_dest, 
            sizeof(addr_dest)) == -1) {
        printf("could not send message\n");
      } else if (msg.type == MSG_MSG) {
        printf("I sent the message\n");
        clock_gettime(CLOCK_REALTIME, &time_wait);
        time_wait.tv_sec += reliable_del ? 2 : 0;
        printf("I will wait for confirmation\n");
        if (sem_timedwait(&ack_sem[msg.destination], &time_wait) == 0) {
          confirmed = 1;
          printf("the message was confirmed\n");
        }
      }
      if (!confirmed) {
        printf("The message was not confirmed\n");
      }
      attempts--;
    }
  }
}

int main(int argc, char **argv) {
  Msg msg;
  pthread_t sender_tid, receiver_tid;

  if (argc < 2) {
    printf("Missing argument\n");
    printf("Correct usage: %s <router id>\n", argv[0]);
    exit(1);
  }
  my_id = atoi(argv[1]);
  my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (my_socket == -1) {
    error("could not create socket");
  }

  setbuf(stdout, NULL);
  set_routers(routers);
  set_network(network);
  set_paths(network, my_id, next_router_to);
  printf("\n");

  for (int i = 0; i < 10; i++) {
    printf("%d id(%d) port(%d) ip(%s)\n", 
        i, routers[i].id, routers[i].port, routers[i].ip);
  }

  sem_init(&receive_sem, 0, 0);
  queue_init(&receive_queue);
  for (int i = 0; i < MAX_ROUTERS; i++) {
    sem_init(&ack_sem[i], 0, 0);
    sem_init(&send_sem[i], 0, 0);
    queue_init(&send_queue[i]);
  }

  pthread_create(&receiver_tid, NULL, receiver_thread, NULL);
  for (int i = 0; i < MAX_ROUTERS; i++) {
    int *arg = (int*)malloc(sizeof(int));
    *arg = i;
    pthread_create(&sender_tid, NULL, sender_thread, (void*)arg);
  }

  while (1) {
    printf("Message content: ");
    scanf("%s", msg.content);
    printf("Destination (router id): ");
    scanf("%d", &msg.destination);
    msg.origin = my_id;
    msg.type = MSG_MSG;

    if (msg.destination == my_id) {
      printf("You can't send a message to yourself\n");
    } else {
      int next_router = next_router_to[msg.destination];
      pthread_mutex_lock(&send_lock[next_router]);
      if (queue_push(&send_queue[next_router], msg)) {
        sem_post(&send_sem[next_router]);
      } else {
        printf("The send queue is full, message discarded\n");
      }

      pthread_mutex_unlock(&send_lock[next_router]);
    }
  }
  return 0;
}
