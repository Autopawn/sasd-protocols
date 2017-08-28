#include <netutils.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>

#include "recvpool.h"

#include <buffer.h>
#include <bvec.h>
#include <errno.h>
#include <assert.h>

void pqueue_init(packet_queue_t* pqueue)
{
    pthread_mutex_init(&pqueue->mutex, NULL);
    pqueue->head = pqueue->tail = NULL;
}

void pqueue_push(packet_queue_t* q, const char* data, size_t size)
{
    pthread_mutex_lock(&q->mutex);

    pnode_t* node = malloc(sizeof(pnode_t));
    node->data = buffer_from_data(data, size);
    node->next = NULL;

    if (q->head == NULL) {
        q->head = q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }

    pthread_mutex_unlock(&q->mutex);
}

struct packet_node* pqueue_pop(packet_queue_t* q)
{
    pthread_mutex_lock(&q->mutex);

    struct packet_node* node = q->head;

    if (q->head != NULL) {
        q->head = q->head->next;

        if (q->head == NULL) {
            q->tail = NULL;
        }
    }

    pthread_mutex_unlock(&q->mutex);

    return node;
}

void _pqueue_destroy_nodes(pnode_t* node)
{
    if (node == NULL) {
        return;
    }
    _pqueue_destroy_nodes(node->next);
    buffer_destroy(node->data);
    free(node);
}

void pqueue_clean(packet_queue_t* pqueue)
{
    _pqueue_destroy_nodes(pqueue->head);
    pthread_mutex_destroy(&pqueue->mutex);
}

#define DATA_DEFAULT_SIZE 1024
#define MIN(x, y) ((x < y) ? x : y)

// TODO: Clean!
void recvpool_init(recvpool_t* pool)
{
    pqueue_init(&pool->pqueue);
}

void* thread(void* arg)
{
    recvpool_t* pool = arg;

    char recv_buffer[RECV_BUFFER_SIZE];
    auto_bvec_t vec = bv_create(RECV_BUFFER_SIZE);

    while (1) {
        int recvd = recv(pool->socket,
            recv_buffer, RECV_BUFFER_SIZE, 0);

        if (recvd == -1) {
            perror("recv");
            continue;
        }

        // Disconnected
        if (recvd == 0) {
            break;
        }

        // Append data to vec
        bv_append(&vec, recv_buffer, recvd);

        while (bv_size(vec) > 2) {
            int16_t packet_size;
            auto_buffer_t size_reader = buffer_from_data(bv_data(vec),
                bv_size(vec));
            buffer_pop_int16(size_reader, &packet_size);

            if (bv_size(vec) >= packet_size) {
                // Push packet to pqueue
                pqueue_push(&pool->pqueue,
                    bv_data(vec), packet_size);
                // Update size of vec
                bv_size(vec) -= packet_size;
                // Move remaining data to beginning
                memmove(bv_data(vec), bv_data(vec) + packet_size,
                    bv_size(vec));
            }
        }
    }
    return NULL;
}

void recvpool_start(recvpool_t* pool, int socket)
{
    if (socket < 0) {
        return;
    }

    recvpool_init(pool);
    pool->socket = socket;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&pool->tid, &attr, thread, pool);

    pthread_attr_destroy(&attr);
}

void recvpool_join(recvpool_t* pool)
{
    pthread_join(pool->tid, NULL);
    close(pool->socket);
    pqueue_clean(&pool->pqueue);
}

int recvpool_retrieve(recvpool_t* pool, packet_t* packet, int parse_sender)
{
    if (packet == NULL) {
        return -1;
    }

    struct packet_node* node = pqueue_pop(&pool->pqueue);

    if (node == NULL) {
        return -1;
    }

    if (packet_deserialize(node->data, packet, parse_sender) == -1) {
        return -1;
    }

    // Clean
    buffer_destroy(node->data);
    free(node);

    return 0;
}
