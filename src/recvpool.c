#include <stdio.h>
#include <stdlib.h>
#include <netutils.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "recvpool.h"

#define DATA_DEFAULT_SIZE 1024
#define MIN(x, y) ((x < y) ? x : y)

void pqueue_init(packet_queue_t *pqueue)
{
    pthread_mutex_init(&pqueue->mutex, NULL);
    pqueue->head = pqueue->tail = NULL;
}

void pqueue_push(packet_queue_t *q, char *d, enum packet_type t)
{
    pthread_mutex_lock(&q->mutex);

    struct packet_node *node = malloc(sizeof(struct packet_node));
    node->data = d; // no copy needed, designed to transfer ownership
    node->ptype = t;
    node->next = NULL;

    if (q->head == NULL) {
        q->head = q->tail = node;
    
    } else {
        q->tail->next = node;
        q->tail = node;
    }

    pthread_mutex_unlock(&q->mutex);
}

struct packet_node *pqueue_pop(packet_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);

    struct packet_node *node = q->head;

    if (q->head != NULL) {
        q->head = q->head->next;

        if (q->head == NULL) {
            q->tail = NULL;
        }
    }

    pthread_mutex_unlock(&q->mutex);

    return node;
}

void pqueue_clean(packet_queue_t *pqueue)
{
    struct packet_node *node;
    for (node = pqueue->head; node != NULL; node = node->next) {
        free(node->data);
    }
    pthread_mutex_destroy(&pqueue->mutex);
}

void pool_reset_data(recvpool_t *pool)
{
    pool->ptype = NONE;
    pool->data = NULL;
    pool->size = DATA_DEFAULT_SIZE;
    pool->len = 0;
}

// TODO: Clean!
void init_pool(recvpool_t *pool)
{
    pqueue_init(&pool->pqueue);
    pool_reset_data(pool);
}

void *thread(void *arg)
{
    recvpool_t *pool = arg;

    while (1) {
        char *buffer = pool->buffer;

        int recvd = recv(pool->socket, buffer, 256, 0);
        
        if (recvd == -1) {
            perror("recv");
            continue;
        }

        // Disconnected
        if (recvd == 0) {
            close(pool->socket);
            break;
        }

        while (recvd > 0) {
            if (pool->ptype == NONE) {
                switch (buffer[0]) {
                    case TEST_PACKET_1:
                    case TEST_PACKET_2:
                        pool->ptype = buffer[0];
                        break;
                    default:
                        printf("Protocol error. Aborting!\n");
                        exit(1);
                }
                // Skip header
                buffer++;
                recvd--;

                pool->data = malloc(DATA_DEFAULT_SIZE);
            }
    
            // Increase heap size if necessary
            if (pool->len + recvd > pool->size) {
                pool->size += pool->size / 2;
                pool->data = realloc(pool->data, pool->size);
            }

            // Copy all received bytes into data
            memcpy(pool->data + pool->len, buffer, recvd);
            size_t rem = get_packet_size(pool->ptype) - pool->len;
            pool->len += recvd;

            if (pool->len >= get_packet_size(pool->ptype)) {
                // Push data into queue
                pqueue_push(&pool->pqueue,
                    pool->data, pool->ptype);

                // Calculate new buffer length as overflowing data
                recvd -= rem;

                // Forward buffer pointer to start at new packet
                buffer += rem;
                // Reset data
                pool_reset_data(pool);
            }
        }
    }
    return NULL;
}

void recvpool_start(recvpool_t *pool, int socket)
{
    if (socket < 0) {
        return;
    }

    init_pool(pool);
    pool->socket = socket;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&pool->tid, &attr, thread, pool);

    pthread_attr_destroy(&attr);
}

void recvpool_clean(recvpool_t *pool)
{
    close(pool->socket);
    free(pool->data);
    pqueue_clean(&pool->pqueue);
    pthread_join(pool->tid, NULL);
}

enum packet_type retrieve(recvpool_t *pool, union packet *packet)
{
    if (packet == NULL) {
        return NONE;
    }

    struct packet_node *node = pqueue_pop(&pool->pqueue);

    if (node == NULL) {
        return NONE;
    }

    // TODO: Change for real serialization !
    switch (node->ptype) {
        case TEST_PACKET_1:
            memcpy(&packet->packet1.data1, node->data, 4);
            packet->packet1.data2 = node->data[4];
            break;
        case TEST_PACKET_2:
            memcpy(&packet->packet2.data1, node->data, 4);
            memcpy(&packet->packet2.data2, node->data + 4, 2);
            break;
        default:
            // UNREACHABLE;
            exit(1);
    }

    free(node->data);
    free(node);

    return node->ptype;
}

