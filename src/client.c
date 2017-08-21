#include <netutils.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "packets.h"
#include "recvpool.h"

#include <string.h>

void close_socket(int* sockref)
{
    if (sockref != NULL) {
        close(*sockref);
    }
}

#define close_on_exit __attribute__((cleanup(close_socket)))

int main(int argc, char* argv[])
{
    int socket = connect_tcp("127.0.0.1", 5000);

    if (socket == -1) {
        perror("Cannot connect");
        exit(1);
    }

    recvpool_t pool;
    recvpool_start(&pool, socket);

    packet_t packet;
    while (recvpool_retrieve(&pool, &packet, 0) == -1)
        ;

    print_packet(&packet);

    while (1) {
        printf("> ");
        struct plain_text plain_text;
        scanf("%s", plain_text.text);

        if (strcmp(plain_text.text, "q!") == 0) {
            break;

        } else if (strcmp(plain_text.text, "r!") == 0) {
            while (recvpool_retrieve(&pool, &packet, 1) != -1) {
                print_packet(&packet);
            }
            continue;
        }
        send_packet(socket, &plain_text, PLAIN_TEXT);
    }

    recvpool_join(&pool);

    return 0;

    /*int socket = connect_tcp("127.0.0.1", 5000);

    if (socket == -1) {
        perror("Cannot connect");
        exit(1);
    }

    recvpool_t pool;
    recvpool_start(&pool, socket);

    int count = 0;
    packet_t packet;

    while (count < 3) {
        if (recvpool_retrieve(&pool, &packet, 0) == -1) {
            continue;
        }

        print_packet(&packet);

        count += 1;
    }

    recvpool_join(&pool);

    return EXIT_SUCCESS;*/
}