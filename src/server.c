#include <buffer.h>
#include <netutils.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "packets.h"

#include <errno.h>
#include <pthread.h>

#include <string.h>

#include <fcntl.h>
#include <sys/select.h>

#include <bvec.h>

void close_socket(int* sockref)
{
    if (sockref != NULL) {
        close(*sockref);
    }
}

#define close_on_exit __attribute__((cleanup(close_socket)))

#define READ_BUFFER_SIZE 1024
#define HBUFFER_SIZE 2048
#define SBUFFER_SIZE 2048

#define panic(msg) \
    perror(msg);   \
    exit(EXIT_FAILURE);

#define log(level, msg, ...) \
    fprintf(stderr, "[" #level "] " msg "\n", ##__VA_ARGS__);

#define MAX(x, y) ((x > y) ? x : y)

struct server_context;

typedef struct __client_node {
    int32_t id;
    int socket;

    // History buffer
    bvec_t hbuffer;
    pthread_mutex_t hbuffer_mutex;

    bvec_t sbuffer;
    pthread_mutex_t sbuffer_mutex;
    // Notification pipe
    int sbuffer_npipe[2];

    // Use it as linked list node
    struct __client_node* next;

    // Reference to server context
    struct server_context* ctx;

    // Its thread id
    pthread_t tid;

} * client_t;

typedef client_t client_list_t;

struct server_context {
    // Context mutex
    pthread_mutex_t mutex;

    int32_t client_count;
    client_list_t clients;
};

client_t client_create(struct server_context* ctx)
{
    client_t client = memset(malloc(sizeof(*client)), 0, sizeof(*client));
    client->ctx = ctx;
    client->hbuffer = bv_create(HBUFFER_SIZE);
    if (pthread_mutex_init(&client->hbuffer_mutex, NULL) != 0) {
        bv_destroy(client->hbuffer);
        free(client);
        return NULL;
    }
    client->sbuffer = bv_create(SBUFFER_SIZE);
    // Init buffer
    if (pthread_mutex_init(&client->sbuffer_mutex, NULL) != 0) {
        bv_destroy(client->hbuffer);
        bv_destroy(client->sbuffer);
        free(client);
        return NULL;
    }
    if (pipe(client->sbuffer_npipe) == -1) {
        free(client);
        return NULL;
    }
    // Set read end of pipe to non-vlock
    fcntl(client->sbuffer_npipe[0], F_SETFL,
        fcntl(client->sbuffer_npipe[0], F_GETFL) | O_NONBLOCK);
    return client;
}

void client_destroy(client_t client)
{
    bv_destroy(client->hbuffer);
    bv_destroy(client->sbuffer);
    close(client->socket);
    pthread_mutex_destroy(&client->hbuffer_mutex);
    pthread_mutex_destroy(&client->sbuffer_mutex);
    close(client->sbuffer_npipe[0]);
    close(client->sbuffer_npipe[1]);
}

void client_notify(client_t client)
{
    write(client->sbuffer_npipe[1], "!", 1);
}

void bv_append_with_sender_id(bvec_t* vec, int32_t sender_id,
    const char* data, int size)
{
    // Read and modify packet size -- it can be made with less mallocs!
    int16_t packet_size;

    auto_buffer_t size_reader = buffer_from_data(data, 2);
    buffer_pop_int16(size_reader, &packet_size);

    packet_size += 4; // Include size of id

    auto_buffer_t prepend_writer = buffer_create(2 + 4);
    buffer_push_int16(prepend_writer, packet_size);
    buffer_push_int32(prepend_writer, sender_id);
    // [size | payload] -> [size | sender_id | payload]

    bv_append(vec, buffer_data(prepend_writer), 2 + 4);
    bv_append(vec, data + 2, size - 2);
}

void server_client_register(struct server_context* ctx, client_t new_client)
{
    pthread_mutex_lock(&ctx->mutex);
    for (client_t client = ctx->clients;
         client != NULL;
         client = client->next) {

        // Send connected packet to the new client
        auto_buffer_t conn_packet = packet_serialize(NULL, CONNECTED);
        bv_append_with_sender_id(&new_client->sbuffer, client->id,
            buffer_data(conn_packet), buffer_size(conn_packet));

        pthread_mutex_lock(&client->hbuffer_mutex);
        bv_append(&new_client->sbuffer,
            bv_data(client->hbuffer),
            bv_size(client->hbuffer));
        pthread_mutex_unlock(&client->hbuffer_mutex);
    }
    // Add client to collection
    new_client->next = ctx->clients;
    ctx->clients = new_client;
    pthread_mutex_unlock(&ctx->mutex);
}

int server_client_shutdown(struct server_context* ctx, int32_t id)
{
    // Send packets
    pthread_mutex_lock(&ctx->mutex);
    for (client_t client = ctx->clients, prev = NULL;
         client != NULL;
         prev = client, client = client->next) {

        if (client->id == id) {
            // If first node
            if (prev == NULL) {
                ctx->clients = client->next;
            } else {
                prev->next = client->next;
            }
            client_destroy(client);
            pthread_mutex_unlock(&ctx->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&ctx->mutex);
    return -1;
}

void server_init(struct server_context* ctx)
{
    pthread_mutex_init(&ctx->mutex, NULL);
    ctx->client_count = 0;
    ctx->clients = NULL;
}

/*
 * Data should always be a full packet!
 */
void server_broadcast(struct server_context* ctx,
    int32_t sender_id, const char* data, int size)
{
    pthread_mutex_lock(&ctx->mutex);
    for (client_t client = ctx->clients;
         client != NULL;
         client = client->next) {

        if (client->id != sender_id) {
            // Append packet stream to client's sbuffer
            pthread_mutex_lock(&client->sbuffer_mutex);
            bv_append_with_sender_id(&client->sbuffer, sender_id, data, size);
            client_notify(client);
            pthread_mutex_unlock(&client->sbuffer_mutex);
        }
    }

    pthread_mutex_unlock(&ctx->mutex);
}

void* client_task(void* data)
{
    client_t client = data;
    // Init read buffer
    char buffer[READ_BUFFER_SIZE];
    //
    auto_bvec_t vec = bv_create(1024);
    int16_t packet_size;

    log(INFO, "Client %d connected", client->id);
    {
        auto_buffer_t con_packet = packet_serialize(NULL, CONNECTED);
        server_broadcast(client->ctx, client->id,
            buffer_data(con_packet), buffer_size(con_packet));
    }

    while (1) {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(client->socket, &fdset);
        FD_SET(client->sbuffer_npipe[0], &fdset);

        int ready = select(
            MAX(client->socket, client->sbuffer_npipe[0]) + 1,
            &fdset,
            NULL,
            NULL,
            NULL);

        if (ready == -1) {
            log(ERROR, "Client %d reports: %s", client->id, strerror(errno));
            continue;
        }

        // If ready to read from socket
        if (FD_ISSET(client->socket, &fdset)) {
            ssize_t rbytes
                = read(client->socket, buffer, READ_BUFFER_SIZE);

            if (rbytes == 0) {
                // Disconnected
                log(INFO, "Client %d disconnected", client->id);
                auto_buffer_t dis_packet = packet_serialize(NULL,
                    DISCONNECTED);
                server_broadcast(client->ctx, client->id,
                    buffer_data(dis_packet), buffer_size(dis_packet));
                server_client_shutdown(client->ctx, client->id);
                break;

            } else if (rbytes == -1) {
                log(ERROR, "Client %d reports: %s", client->id, strerror(errno));
                continue;
            }

            // Clean code, but too many copies for my taste D:
            bv_append(&vec, buffer, rbytes);
            while (bv_size(vec) > 2) {
                // Get size of packet
                auto_buffer_t size_reader = buffer_from_data(bv_data(vec), 2);
                buffer_pop_int16(size_reader, &packet_size);

                // Prepend sender
                auto_buffer_t prepend_writer = buffer_create(2 + 4);
                buffer_push_int16(prepend_writer, packet_size + 4);
                buffer_push_int32(prepend_writer, client->id);

                if (bv_size(vec) >= packet_size) {
                    // Put a packet_size sized stream in the send queues and
                    // history buffer to ensure integrity!!!
                    pthread_mutex_lock(&client->hbuffer_mutex);
                    bv_append_with_sender_id(&client->hbuffer, client->id,
                        bv_data(vec), packet_size);
                    pthread_mutex_unlock(&client->hbuffer_mutex);

                    server_broadcast(client->ctx,
                        client->id, bv_data(vec), packet_size);

                    // Move remaining data into accumulator
                    memmove(vec, vec + packet_size, bv_size(vec) - packet_size);
                    bv_size(vec) = bv_size(vec) - packet_size;
                }
            }
        }
        // If there is data to send
        if (FD_ISSET(client->sbuffer_npipe[0], &fdset)) {
            pthread_mutex_lock(&client->sbuffer_mutex);
            // Consume data in notification pipe
            char discard[10];
            while (read(client->sbuffer_npipe[0], discard, 10) != -1)
                ;
            // errno expected to be EAGAIN
            if (errno == EAGAIN) {
                int to_be_sent = bv_size(client->sbuffer);
                int sent = 0;

                while (to_be_sent != sent) {
                    int ret_val = send(client->socket,
                        bv_data(client->sbuffer) + sent,
                        bv_size(client->sbuffer) - sent, 0);

                    if (ret_val == -1) {
                        log(ERROR, "Client %d reports error while sending: %s",
                            client->id, strerror(errno));
                        break;
                    }
                    // Update sent count
                    sent += ret_val;
                }
                // Can overwrite data
                bv_size(client->sbuffer) = 0;
            } else {
                log(ERROR, "Client %d reports unexpected error: %s",
                    client->id, strerror(errno));
            }
            pthread_mutex_unlock(&client->sbuffer_mutex);
        }
    }

    return NULL;
}

void server_start(uint16_t port)
{
    struct server_context ctx;
    server_init(&ctx);

    int socket close_on_exit = listen_tcp(port);
    if (socket == -1) {
        panic("Could not start server");
    }

    while (1) {
        struct sockaddr addr;
        socklen_t size = sizeof(addr);

        client_t client = client_create(&ctx);
        if (client == NULL) {
            log(CRITICAL, "Could not init client: %s", strerror(errno));
            continue;
        }

        client->socket = accept(socket, &addr, &size);

        if (client->socket == -1) {
            log(ERROR, "Could not accept connection: %s", strerror(errno));
            continue;
        }

        // Assign id to client and send it as handshake
        client->id = ctx.client_count++;
        struct handshake handshake;
        handshake.client_id = client->id;
        auto_buffer_t hdata = packet_serialize(&handshake, HANDSHAKE);
        bv_append(&client->sbuffer, buffer_data(hdata), buffer_size(hdata));
        client_notify(client);

        /*char id_buffer[4];
        memcpy(id_buffer, &client->id, 4);
        if (send(client->socket, id_buffer, 4, 0) == -1) {
            log(ERROR, "Could not send handshake: %s", strerror(errno));
            continue;
        }*/

        // Add client to list
        server_client_register(&ctx, client);

        // Start client thread
        pthread_create(&client->tid, NULL, client_task, client);
    }
    // Exit threads
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    server_start(5000);
    return 0;

    int socket close_on_exit = listen_tcp(5000);
    if (socket == -1) {
        perror("Cannot listen");
        exit(1);
    }

    struct sockaddr addr;
    socklen_t size = sizeof(addr);
    int client close_on_exit = accept(socket, &addr, &size);

    if (client == -1) {
        perror("Cannot accept");
        exit(1);
    }

    struct test_packet_1 packet1;
    struct test_packet_2 packet2;

    packet1.data1 = 42;
    packet1.data2 = 'g';

    send_packet(client, &packet1, TEST_PACKET_1);

    packet1.data1 = 31;
    packet1.data2 = 'h';

    send_packet(client, &packet1, TEST_PACKET_1);

    packet2.data1 = 40;
    packet2.data2 = 2;

    send_packet(client, &packet2, TEST_PACKET_2);

    return EXIT_SUCCESS;
}