#ifndef PACKETS_H
#define PACKETS_H

#include <buffer.h>
#include <stdint.h>
#include <stdlib.h>

struct test_packet_1 {
    int32_t data1;
    char data2;
};

struct test_packet_2 {
    int32_t data1;
    int16_t data2;
};

struct handshake {
    int32_t client_id;
};

struct plain_text {
    char text[256];
};

enum payload_type {
    NONE = '-',
    TEST_PACKET_1 = 'a',
    TEST_PACKET_2 = 'b',
    HANDSHAKE = 'h',
    PLAIN_TEXT = 't',
    DISCONNECTED = 'd',
    CONNECTED = 'c'
};

union payload {
    struct test_packet_1 packet1;
    struct test_packet_2 packet2;
    struct handshake handshake;
    struct plain_text plain_text;
};

typedef struct {
    int16_t size;
    int32_t sender_id;
    enum payload_type ptype;
    union payload payload;

} packet_t;

int packet_deserialize(buffer_t data, packet_t* packet, int parse_sender);
buffer_t packet_serialize(const void*, enum payload_type);

void send_packet(int socket, const void* payload, enum payload_type);

void print_packet(const packet_t* packet);

#endif