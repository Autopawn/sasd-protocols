#ifndef RECVPOOL_h
#define RECVPOOL_h

#include "packets.h"
#include <pthread.h>

#define RECV_BUFFER_SIZE 1024

typedef struct packet_node {
    struct packet_node* next;
    buffer_t data;

} pnode_t;

typedef struct
{
    pthread_mutex_t mutex;
    pnode_t* head;
    pnode_t* tail;

} packet_queue_t;

typedef struct {
    int socket;
    pthread_t tid;
    packet_queue_t pqueue;

} recvpool_t;

void recvpool_init(recvpool_t* pool);
void recvpool_start(recvpool_t* pool, int socket);
void recvpool_join(recvpool_t* pool);
int recvpool_retrieve(recvpool_t* pool, packet_t* packet, int parse_sender);

#endif