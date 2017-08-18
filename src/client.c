#include <stdio.h>
#include <stdlib.h>
#include <netutils.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "packets.h"
#include "recvpool.h"


void close_socket(int *sockref)
{
    if (sockref != NULL) {
        close(*sockref);
    }
}

#define close_on_exit __attribute__((cleanup (close_socket)))

int main(int argc, char *argv[])
{
    int socket = connect_tcp("127.0.0.1", 5000);

    if (socket == -1) {
        perror("Cannot connect");
        exit(1);
    }

    recvpool_t pool;
    recvpool_start(&pool, socket);

    int count = 0;
    union packet packet;

    while (count < 3) {
        enum packet_type ptype = retrieve(&pool, &packet);

        if (ptype == NONE) {
            continue;
        }

        print_packet(&packet, ptype);

        count += 1;
    }

    recvpool_clean(&pool);

    /*if (send(socket, "Hi!", 3, 0) == -1) {
        perror("Cannot send");
        exit(1);
    }*

    char buffer[256];
    int received = recv(socket, buffer, 256, 0);
    
    if (received == -1) {
        perror("Cannot recv");
        exit(1);
    }

    buffer[received] = 0;
    printf("%s\n", buffer);*/
}