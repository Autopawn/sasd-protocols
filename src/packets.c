#include "packets.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <string.h>

size_t get_packet_size(enum packet_type ptype)
{
    switch (ptype) {
        case NONE:
            return 0;
        case TEST_PACKET_1:
            return 5;
        case TEST_PACKET_2:
            return 6;
        default:
            printf("get_packet_size");
            exit(1);
    }
}

void send_packet1(int socket, const struct test_packet_1 *packet)
{
    char buffer[256];

    buffer[0] = TEST_PACKET_1;
    memcpy(buffer + 1, &packet->data1, 4);
    buffer[5] = packet->data2;

    send(socket, buffer, get_packet_size(TEST_PACKET_1) + 1, 0);
}

void send_packet2(int socket, const struct test_packet_2 *packet)
{
    char buffer[256];
    
    buffer[0] = TEST_PACKET_2;
    memcpy(buffer + 1, &packet->data1, 4);
    memcpy(buffer + 5, &packet->data2, 2);

    send(socket, buffer, get_packet_size(TEST_PACKET_2) + 1, 0);
}

void print_packet(const union packet *packet, enum packet_type ptype)
{
    switch (ptype) {
        case NONE:
            printf("NONE\n");
            break;
        case TEST_PACKET_1:
            printf("test_packet_1 {\n");
            printf("\tdata1 = %d\n", packet->packet1.data1);
            printf("\tdata2 = %c\n}\n", packet->packet1.data2);
            break;
        case TEST_PACKET_2:
            printf("test_packet_2 {\n");
            printf("\tdata1 = %d\n", packet->packet2.data1);
            printf("\tdata2 = %d\n}\n", packet->packet2.data2);
            break;
        default:
            printf("UNKNOWN\n");
        }
}