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

#define MAX_ROUTERS 20
#define ROUTERS_FILE "roteador.config"
#define LINKS_FILE "enlaces.config"

#define IP_STR_LEN 20

pthread_mutex_t send_lock[MAX_ROUTERS], receive_lock, output_lock;
sem_t send_sem[MAX_ROUTERS], receive_sem;
sem_t ack_sem[MAX_ROUTERS];
struct msg_queue send_queue[MAX_ROUTERS], receive_queue;
int my_id, my_socket;

struct router {
  int id;
  int port;
  char ip[IP_STR_LEN];
};

/**
 * matriz de adjacência da rede
 * se o valor for nao negativo, representa o peso da aresta
 * se for -1, nao existe enlace
 */
int network[MAX_ROUTERS][MAX_ROUTERS];

struct router routers[MAX_ROUTERS];

void error(const char *str) {
  fprintf(stderr, "Error: %s\n", str);
  exit(1);
}

void set_routers(void) {
  int id, port;
  char ip[IP_STR_LEN];
  FILE *file = fopen(ROUTERS_FILE, "r");

  assert(file != NULL);

  while (fscanf(file, "%d %d %s", &id, &port, ip) == 3) {
    routers[id].id = id;
    routers[id].port = port;
    strcpy(routers[id].ip, ip);
  }

  fclose(file);
}

void set_network(void) {
  int id1, id2, weight;
  FILE *file = fopen(LINKS_FILE, "r");

  assert(file != NULL);

  memset(network, -1, MAX_ROUTERS * MAX_ROUTERS);
  while (fscanf(file, "%d %d %d", &id1, &id2, &weight) == 3) {
    assert(id1 < MAX_ROUTERS);
    assert(id2 < MAX_ROUTERS);
    assert(weight >= 0);
    network[id1][id2] = network[id2][id1] = weight;
  }
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
      pthread_mutex_lock(&send_lock[msg.destination]);
      queue_push(&send_queue[msg.destination], msg);
      pthread_mutex_unlock(&send_lock[msg.destination]);
    } else if (msg.type == MSG_ACK) { // confirmation
      printf("I got an ACK\n");
      sem_post(&ack_sem[msg.origin]);
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

    addr_dest.sin_port = htons((uint16_t)routers[msg.destination].port);
    printf("the destination is %d\n", msg.destination);
    if (inet_aton(routers[msg.destination].ip, 
          &addr_dest.sin_addr) == -1) {
      error("inet_aton FAILED in sender_thread");
    }

    attempts = msg.type == MSG_MSG ? 3 : 1;
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
        time_wait.tv_sec += 2;
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
  set_routers();
  printf("\n");
  set_network();

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

    pthread_mutex_lock(&send_lock[msg.destination]);
    if (queue_push(&send_queue[msg.destination], msg)) {
      sem_post(&send_sem[msg.destination]);
    } else {
      printf("The send queue is full, message discarded\n");
    }

    pthread_mutex_unlock(&send_lock[msg.destination]);
  }
  return 0;
}
