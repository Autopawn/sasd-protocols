#include "packets.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>

#define TRY(ret)   \
    if (ret == -1) \
        return -1;

int _test_packet_1_deserialize(buffer_t data, struct test_packet_1* payload)
{
    TRY(buffer_pop_int32(data, &payload->data1));
    TRY(buffer_pop_char(data, &payload->data2));
    return 0;
}

int _test_packet_2_deserialize(buffer_t data, struct test_packet_2* payload)
{
    TRY(buffer_pop_int32(data, &payload->data1));
    TRY(buffer_pop_int16(data, &payload->data2));
    return 0;
}

int _handshake_deserialize(buffer_t data, struct handshake* payload)
{
    TRY(buffer_pop_int32(data, &payload->client_id));
    return 0;
}

int _plain_text_deserialize(buffer_t data, struct plain_text* payload)
{
    TRY(buffer_pop_string(data, &payload->text, 256));
    return 0;
}

int _ball_deserialize(buffer_t data, ball* payload)
{
    TRY(buffer_pop_int32(data, &payload->p_x));
    TRY(buffer_pop_int32(data, &payload->p_y));
    TRY(buffer_pop_int32(data, &payload->v_x));
    TRY(buffer_pop_int32(data, &payload->v_y));
    TRY(buffer_pop_int32(data, &payload->player));

    for (int i = 0; i < N_BUTTONS; i++) {
        TRY(buffer_pop_char(data, &payload->button_states[i]));
    }
    return 0;
}

int _state_deserialize(buffer_t data, state* payload)
{
    TRY(buffer_pop_int32(data, &payload->frame));
    for (int i = 0; i < MAX_PLAYERS; i++) {
        TRY(buffer_pop_int32(data, &payload->scores[i]));
    }
    for (int i = 0; i < N_BALLS; i++) {
        TRY(_ball_deserialize(data, &payload->balls[i]));
    }
    return 0;
}

int _event_deserialize(buffer_t data, event* payload)
{
    TRY(buffer_pop_int32(data, &payload->frame));
    TRY(buffer_pop_int32(data, &payload->summon_frame));
    TRY(buffer_pop_int32(data, &payload->player));
    TRY(buffer_pop_char(data, &payload->button));
    TRY(buffer_pop_char(data, &payload->pressed));
    return 0;
}

int packet_deserialize(buffer_t data, packet_t* packet, int parse_sender)
{
    TRY(buffer_pop_int16(data, &packet->size));

    if (parse_sender) {
        TRY(buffer_pop_int32(data, &packet->sender_id));
    }
    char ptype;
    TRY(buffer_pop_char(data, &ptype));
    packet->ptype = ptype;

    switch (ptype) {
    case TEST_PACKET_1:
        TRY(_test_packet_1_deserialize(data, &packet->payload.packet1));
        break;
    case TEST_PACKET_2:
        TRY(_test_packet_2_deserialize(data, &packet->payload.packet2));
        break;
    case HANDSHAKE:
        TRY(_handshake_deserialize(data, &packet->payload.handshake));
        break;
    case PLAIN_TEXT:
        TRY(_plain_text_deserialize(data, &packet->payload.plain_text));
        break;
    case DISCONNECTED:
        // No payload
        break;
    case CONNECTED:
        // No payload
        break;
    case BALL:
        TRY(_ball_deserialize(data, &packet->payload.ball));
        break;
    case STATE:
        TRY(_state_deserialize(data, &packet->payload.state));
        break;
    case EVENT:
        TRY(_event_deserialize(data, &packet->payload.event));
        break;
    }
    return 0;
}

buffer_t _header_serialize(int16_t psize, enum payload_type ptype)
{
    // size + type + payload_size
    buffer_t buffer = buffer_create(2 + 1 + psize);
    buffer_push_int16(buffer, 2 + 1 + psize);
    buffer_push_char(buffer, ptype);
    return buffer;
}

buffer_t _test_packet_1_serialize(const struct test_packet_1* payload)
{
    // int32 + char
    buffer_t buffer = _header_serialize(4 + 1, TEST_PACKET_1);
    buffer_push_int32(buffer, payload->data1);
    buffer_push_char(buffer, payload->data2);
    return buffer;
}

buffer_t _test_packet_2_serialize(const struct test_packet_2* payload)
{
    // int32 + int16
    buffer_t buffer = _header_serialize(4 + 2, TEST_PACKET_2);
    buffer_push_int32(buffer, payload->data1);
    buffer_push_int16(buffer, payload->data2);
    return buffer;
}

buffer_t _handshake_serialize(const struct handshake* payload)
{
    // int32
    buffer_t buffer = _header_serialize(4, HANDSHAKE);
    buffer_push_int32(buffer, payload->client_id);
    return buffer;
}

buffer_t _plain_text_serialize(const struct plain_text* payload)
{
    // string_size + var
    buffer_t buffer = _header_serialize(2 + strlen(payload->text), PLAIN_TEXT);
    buffer_push_string(buffer, payload->text);
    return buffer;
}

buffer_t _disconnected_serialize()
{
    // no payload
    buffer_t buffer = _header_serialize(0, DISCONNECTED);
    return buffer;
}

buffer_t _connected_serialize()
{
    // no payload
    buffer_t buffer = _header_serialize(0, CONNECTED);
    return buffer;
}

buffer_t _ball_serialize(const ball* payload, buffer_t buffer)
{
    // int32 * 5 + char * N_BUTTONS
    if (buffer == NULL) {
        buffer = _header_serialize(20 + N_BUTTONS, BALL);
    }

    buffer_push_int32(buffer, payload->p_x);
    buffer_push_int32(buffer, payload->p_y);
    buffer_push_int32(buffer, payload->v_x);
    buffer_push_int32(buffer, payload->v_y);
    buffer_push_int32(buffer, payload->player);

    for (int i = 0; i < N_BUTTONS; i++) {
        buffer_push_char(buffer, payload->button_states[i]);
    }

    return buffer;
}

buffer_t _state_serialize(const state* payload)
{
    // int32 + int32 * MAX_PLAYERS + (ball = 20 + N_BUTTONS) * MAX_PLAYERS
    buffer_t buffer = _header_serialize(4 + (4 * MAX_PLAYERS) + ((20 + N_BUTTONS) * N_BALLS), STATE);
    buffer_push_int32(buffer, payload->frame);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        buffer_push_int32(buffer, payload->scores[i]);
    }
    for (int i = 0; i < N_BALLS; i++) {
        _ball_serialize(&payload->balls[i], buffer);
    }
    return buffer;
}

buffer_t _event_serialize(const event* payload)
{
    // 3 * int32 + 2 * char
    buffer_t buffer = _header_serialize(12 + 2, EVENT);
    buffer_push_int32(buffer, payload->frame);
    buffer_push_int32(buffer, payload->summon_frame);
    buffer_push_int32(buffer, payload->player);
    buffer_push_char(buffer, payload->button);
    buffer_push_char(buffer, payload->pressed);
    return buffer;
}

buffer_t packet_serialize(const void* payload,
    enum payload_type ptype)
{
    switch (ptype) {
    case TEST_PACKET_1:
        return _test_packet_1_serialize(payload);
    case TEST_PACKET_2:
        return _test_packet_2_serialize(payload);
    case HANDSHAKE:
        return _handshake_serialize(payload);
    case PLAIN_TEXT:
        return _plain_text_serialize(payload);
    case DISCONNECTED:
        // Payload ignored
        return _disconnected_serialize();
    case CONNECTED:
        // Payload ignored
        return _connected_serialize();
    case BALL:
        return _ball_serialize(payload, NULL);
    case STATE:
        return _state_serialize(payload);
    case EVENT:
        return _event_serialize(payload);
    default:
        return NULL;
    }
}

void send_packet(int socket, const void* payload, enum payload_type ptype)
{
    auto_buffer_t data = packet_serialize(payload, ptype);
    send(socket, buffer_data(data), buffer_size(data), 0);
}

void print_packet(const packet_t* packet)
{
    packet_t child;

    switch (packet->ptype) {
    case NONE:
        printf("NONE\n");
        break;
    case TEST_PACKET_1:
        printf("test_packet_1 {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tdata1 = %d\n", packet->payload.packet1.data1);
        printf("\tdata2 = %c\n}\n", packet->payload.packet1.data2);
        break;
    case TEST_PACKET_2:
        printf("test_packet_2 {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tdata1 = %d\n", packet->payload.packet2.data1);
        printf("\tdata2 = %d\n}\n", packet->payload.packet2.data2);
        break;
    case HANDSHAKE:
        printf("handshake {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tclient_id = %d\n}\n", packet->payload.handshake.client_id);
        break;
    case PLAIN_TEXT:
        printf("plain_text {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\ttext = %s\n}\n", packet->payload.plain_text.text);
        break;
    case DISCONNECTED:
        printf("disconnected {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n}\n", packet->sender_id);
        break;
    case CONNECTED:
        printf("connected {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n}\n", packet->sender_id);
        break;
    case BALL:
        printf("ball {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tp_x = %d\n", packet->payload.ball.p_x);
        printf("\tp_y = %d\n", packet->payload.ball.p_y);
        printf("\tv_x = %d\n", packet->payload.ball.v_x);
        printf("\tv_y = %d\n", packet->payload.ball.v_y);
        printf("\tplayer = %d\n", packet->payload.ball.player);
        printf("\tbutton_states = [");
        for (int i = 0; i < N_BUTTONS - 1; i++) {
            printf("%c, ", packet->payload.ball.button_states[i]);
        }
        printf("%c]\n}\n", packet->payload.ball.button_states[N_BUTTONS - 1]);
        break;
    case STATE:
        printf("state {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tframe = %d\n", packet->payload.state.frame);
        printf("\tscores = [");
        for (int i = 0; i < MAX_PLAYERS - 1; i++) {
            printf("%d, ", packet->payload.state.scores[i]);
        }
        printf("%d]\n", packet->payload.state.scores[MAX_PLAYERS - 1]);
        printf("\tballs = [");
        for (int i = 0; i < N_BALLS - 1; i++) {
            child.size = 0;
            child.sender_id = packet->sender_id;
            child.ptype = BALL;
            child.payload.ball = packet->payload.state.balls[i];
            print_packet(&child);
            printf(", ");
        }
        child.size = 0;
        child.sender_id = packet->sender_id;
        child.ptype = BALL;
        child.payload.ball = packet->payload.state.balls[N_BALLS - 1];
        print_packet(&child);
        printf("]\n}\n");
        break;
    case EVENT:
        printf("event {\n");
        printf("\tsize = %d\n", packet->size);
        printf("\tsender_id = %d\n", packet->sender_id);
        printf("\tframe = %d\n", packet->payload.event.frame);
        printf("\tsummon_frame = %d\n", packet->payload.event.summon_frame);
        printf("\tplayer = %d\n", packet->payload.event.player);
        printf("\tbutton = %c\n", packet->payload.event.button);
        printf("\tpressed = %d\n}\n", (int)packet->payload.event.pressed);
        break;
    default:
        printf("UNKNOWN\n");
    }
}