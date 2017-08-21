#ifndef RECVPOOL_h
#define RECVPOOL_h

#include <pthread.h>
#include "packets.h"

struct packet_node
{
    char *data;
    enum packet_type ptype;
    struct packet_node *next;
};

typedef struct
{
    pthread_mutex_t mutex;
    struct packet_node *head;
    struct packet_node *tail;

} packet_queue_t;

typedef struct
{
    int socket;
    pthread_t tid;

    char buffer[256];

    enum packet_type ptype;
    char *data;
    size_t size;
    size_t len; // data length

    packet_queue_t pqueue;

} recvpool_t;

void pqueue_init(packet_queue_t *pqueue);
void pqueue_push(packet_queue_t *q, char *d, enum packet_type t);
struct packet_node *pqueue_pop(packet_queue_t *q);
void pqueue_clean(packet_queue_t *pqueue);
void pool_reset_data(recvpool_t *pool);
void init_pool(recvpool_t *pool);
void *thread(void *arg);
void recvpool_start(recvpool_t *pool, int socket);
void recvpool_clean(recvpool_t *pool);
enum packet_type retrieve(recvpool_t *pool, union packet *packet);

#endif