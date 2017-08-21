#ifndef PACKETS_H
#define PACKETS_H

#include <stdlib.h>
#include <stdint.h>

struct test_packet_1
{
    int32_t data1;
    char data2;
};

struct test_packet_2
{
    int32_t data1;
    int16_t data2;
};

enum packet_type {
    NONE = '-',
    TEST_PACKET_1 = 'a',
    TEST_PACKET_2 = 'b'
};

union packet
{
    struct test_packet_1 packet1;
    struct test_packet_2 packet2;
};

size_t get_packet_size(enum packet_type ptype);

void send_packet1(int socket, const struct test_packet_1 *);
void send_packet2(int socket, const struct test_packet_2 *);

void print_packet(const union packet *, enum packet_type);

#endif