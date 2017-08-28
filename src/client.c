#include <netutils.h>
#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "packets.h"
#include "recvpool.h"

#include <string.h>

#include "balls/balls.h"

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
    // Receive handshake
    while (recvpool_retrieve(&pool, &packet, 0) == -1)
        ;

    print_packet(&packet);
    int32_t player_id = packet.payload.handshake.client_id;

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

        } else if (strcmp(plain_text.text, "b!") == 0) {
            ball b;
            b.p_x = 0;
            b.p_y = 1;
            b.v_x = 2;
            b.v_y = 3;
            b.player = player_id;
            char a = 'a';
            for (int i = 0; i < N_BUTTONS; i++) {
                b.button_states[i] = a++;
            }
            send_packet(socket, &b, BALL);
            continue;
        } else if (strcmp(plain_text.text, "s!") == 0) {
            state s;
            s.frame = 42;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                s.scores[i] = i;
            }
            ball b;
            b.p_x = 0;
            b.p_y = 1;
            b.v_x = 2;
            b.v_y = 3;
            b.player = player_id;
            char a = 'a';
            for (int i = 0; i < N_BUTTONS; i++) {
                b.button_states[i] = a++;
            }
            for (int i = 0; i < N_BALLS; i++) {
                s.balls[i] = b;
                b.p_x++;
                b.p_y++;
                b.v_x++;
                b.v_y++;
                b.player++;
                for (int i = 0; i < N_BUTTONS; i++) {
                    b.button_states[i]++;
                }
            }
            send_packet(socket, &s, STATE);
            continue;

        } else if (strcmp(plain_text.text, "e!") == 0) {
            event e;
            e.frame = 31;
            e.summon_frame = 29;
            e.player = player_id;
            e.button = 'g';
            e.pressed = 1;
            send_packet(socket, &e, EVENT);
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
