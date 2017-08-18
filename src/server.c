#include <stdio.h>
#include <stdlib.h>
#include <netutils.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "packets.h"

void close_socket(int *sockref)
{
    if (sockref != NULL) {
        close(*sockref);
    }
}

#define close_on_exit __attribute__((cleanup (close_socket)))

int main(int argc, char *argv[])
{
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

    send_packet1(client, &packet1);

    packet1.data1 = 31;
    packet1.data2 = 'h';
    
    send_packet1(client, &packet1);

    packet2.data1 = 40;
    packet2.data2 = 2;

    send_packet2(client, &packet2);

    /*char buffer[256];
    int received = recv(client, buffer, 256, 0);
    
    if (received == -1) {
        perror("Cannot recv");
        exit(1);
    }

    buffer[received] = 0;
    printf("Received: %s\n", buffer);

    if (send(client, "Bye", 3, 0) == -1) {
        perror("Cannot send");
        exit(1);
    }*/

    return 0;
}