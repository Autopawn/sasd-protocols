#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define SCREEN_SCALE 1
#define SCREEN_NAME "sasd-protocols fast-event-ordering"

#include "balls/balls.h"
#include "balls/draw.h"

#include "packets.h"
#include "recvpool.h"
#include "vec.h"

#include <netutils.h>
#include <balls/balls.h>

static int lags[MAX_PLAYERS][MAX_PLAYERS] = {
    { 0,  0, 30, 0, 0, 0, 0, 0},
    { 0,  0, 30, 0, 0, 0, 0, 0},
    {30, 30,  0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0},
    { 0,  0,  0, 0, 0, 0, 0, 0}
};

static int n_clients;
static int delays[MAX_PLAYERS][MAX_PLAYERS];

int compute_delays(){
    printf("clients: %d\n",n_clients);
    int difs[MAX_PLAYERS];
    for(int p_ref=n_clients-1;p_ref>=0;p_ref--){
        for(int i=0;i<n_clients;i++) difs[i]=0;
        for(int p_i=0;p_i<n_clients;p_i++){
            if(p_i==p_ref) continue;
            int max_dif = -1;
            for(int p_j=0;p_j<n_clients;p_j++){
                int dif_temp = lags[p_j][p_i]-lags[p_j][p_ref];
                if(dif_temp>max_dif) max_dif = dif_temp;
            }
            difs[p_i] = max_dif;
        }
        //
        int min_dif = difs[0];
        for(int i=1;i<n_clients;i++){
            if(difs[i]<min_dif) min_dif = difs[i];
        }
        printf("min_dif: %d\n",min_dif);
        if(min_dif>0) continue;
        // Calculate and print the adjusts for each node.
        printf("Adjusts:\n");
        for(int myid=0;myid<n_clients;myid++){
            for(int p_i=0;p_i<n_clients;p_i++){
                int adjust =  difs[p_i]-(lags[myid][p_i]-lags[myid][p_ref]);
                delays[myid][p_i] = lags[myid][p_i]+adjust;
                printf("%4d",delays[myid][p_i]);
            }
            printf("\n");
        }
        return 1;
    }
    return 0;
}

struct {
    int running;

    struct {
        unsigned int w;
        unsigned int h;
        const char* name;
        SDL_Window* window;
        SDL_Renderer* renderer;
    } screen;

} game_state = {
    .running = 1,

    .screen = {
        .w = SCREEN_W,
        .h = SCREEN_H,
        .name = SCREEN_NAME,
        .window = NULL,
        .renderer = NULL }
};

void game_init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL error: %s\n", SDL_GetError());
        exit(1);
    }

    game_state.screen.window = SDL_CreateWindow(game_state.screen.name,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        game_state.screen.w, game_state.screen.h, 0);
    game_state.screen.renderer = SDL_CreateRenderer(game_state.screen.window,
        -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void game_quit()
{
    SDL_DestroyRenderer(game_state.screen.renderer);
    SDL_DestroyWindow(game_state.screen.window);

    game_state.screen.window = NULL;
    game_state.screen.renderer = NULL;

    SDL_Quit();
    game_state.running = 0;
}

typedef struct {
    int count;
    int wptr;
    event evs[];

} *ev_vec_t;

ev_vec_t ev_vec_create()
{
    ev_vec_t ev_vec = malloc(sizeof(*ev_vec) + 8 * sizeof(event));
    ev_vec->count = 8;
    ev_vec->wptr = 0;
    return ev_vec;
}

void ev_vec_destroy(ev_vec_t ev_vec)
{
    free(ev_vec);
}

void ev_vec_push(ev_vec_t *ev_vec_ref, event ev)
{
    ev_vec_t ev_vec = *ev_vec_ref;

    if (ev_vec->wptr == ev_vec->count) {
        ev_vec->count += ev_vec->count / 2;
        ev_vec = *ev_vec_ref = realloc(ev_vec,
            sizeof(*ev_vec) + ev_vec->count * sizeof(event));
    }

    ev_vec->evs[ev_vec->wptr++] = ev;
}

// CLEAN!
int main(int argc, char* argv[])
{
    if(argc!=2){
        printf("Usage: %s <n_clients>\n",argv[0]);
        exit(1);
    }
    printf("argv1 %s\n",argv[1]);
    sscanf(argv[1],"%d",&n_clients);
    if(!compute_delays()){
        printf("Algorithm failed!\n");
        exit(1);
    }

    game_init();

    // Initial state
    state stat = initial_state(0);

    int trace_size = 512;

    auto_vec_t state_trace = vec_create(trace_size, sizeof(state), 0);
    auto_vec_t event_trace = vec_create(trace_size, sizeof(ev_vec_t), 1);
    auto_vec_t event_buffer = vec_create(trace_size, sizeof(ev_vec_t), 1);

    vec_set(&state_trace, 0, &stat);

    int min_frame = 1;
    int frame = 1;

    event ev;

    int socket = connect_tcp("127.0.0.1", 5000);
    if (socket == -1) {
        perror("Could not connect to server");
        exit(1);
    }
    packet_t packet;

    recvpool_t pool;
    recvpool_start(&pool, socket);

    while (recvpool_retrieve(&pool, &packet, 0) == -1)
        ;

    if (packet.ptype != HANDSHAKE) {
        printf("Protocol error. Aborting.");
        return 1;
    }

    int32_t player = packet.payload.handshake.client_id;
	char title[256];
	sprintf(title, "%s client: %d", game_state.screen.name, player);
	SDL_SetWindowTitle(game_state.screen.window, title);

    event connect;
    connect.frame = frame+delays[player][player];
    // ^ On FEO the execution frame is not controlled by the client.
    connect.summon_frame = frame;
    connect.button = N_BUTTONS;
    connect.pressed = 1;
	connect.player = player;

    ev_vec_t ev_vec = ev_vec_create();
    vec_set(&event_trace, connect.frame, &ev_vec);
    ev_vec_push(&ev_vec, connect);

    send_packet(socket, &connect, EVENT);

    SDL_Event sdl_event;

    Uint32 game_start = SDL_GetTicks();
    while (game_state.running) {
        vec_set(&state_trace, frame, &stat);

        if (vec_get(event_trace, frame + delays[player][player], &ev_vec) == -1 || ev_vec == 0) {
            ev_vec = ev_vec_create();
            vec_set(&event_trace, frame + delays[player][player], &ev_vec);
        }

        while (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type) {
            case SDL_QUIT:
                game_state.running = 0;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ev.frame = frame + delays[player][player];
                ev.summon_frame = frame;
                ev.player = player;
                ev.pressed = (sdl_event.type == SDL_KEYDOWN);

                int good = 1;
                switch (sdl_event.key.keysym.sym) {
                case SDLK_RIGHT:
                    ev.button = 0;
                    break;
                case SDLK_UP:
                    ev.button = 1;
                    break;
                case SDLK_LEFT:
                    ev.button = 2;
                    break;
                case SDLK_DOWN:
                    ev.button = 3;
                    break;
                default:
                    good = 0;
                    break;
                }
                if (good) {
                    ev_vec_push(&ev_vec, ev);
                }
                break;
            }
        }

        // Broadcast events
        for (int i = 0; i < ev_vec->wptr; i++) {
	        if (ev_vec->evs[i].player == player) {
		        send_packet(socket, ev_vec->evs + i, EVENT);
	        }
        }

        // Receive events
        while (recvpool_retrieve(&pool, &packet, 1) != -1) {
            if (packet.ptype == EVENT) {
                event remote_ev = packet.payload.event;
                // FEO Requires that the execution frame is set to this one:
                remote_ev.frame = remote_ev.summon_frame +
                    delays[player][remote_ev.player];
                int q = remote_ev.summon_frame + lags[player][remote_ev.player];
                q = (lags[player][remote_ev.player] == 0 || q < frame) ? frame : q;

                if (vec_get(event_buffer, q, &ev_vec) == -1 || ev_vec == 0) {
                    ev_vec = ev_vec_create();
                    vec_set(&event_buffer, q, &ev_vec);
                }
                ev_vec_push(&ev_vec, remote_ev);
            }
        }

        if (vec_get(event_buffer, frame, &ev_vec) != -1 && ev_vec != 0) {
            for (int i = 0; i < ev_vec->wptr; i++) {
                ev_vec_t trace_ev;
                event remote_ev = ev_vec->evs[i];

                if (vec_get(event_trace, remote_ev.frame, &trace_ev) == -1 || trace_ev == 0) {
                    trace_ev = ev_vec_create();
                    vec_set(&event_trace, remote_ev.frame, &trace_ev);
                }
                ev_vec_push(&trace_ev, remote_ev);

                if (remote_ev.frame < min_frame) {
                    min_frame = remote_ev.frame;
                }
            }
        }

        // Synchronize
        for (int i = min_frame; i <= frame; i++) {
            vec_get(state_trace, i - 1, &stat);
            if (vec_get(event_trace, i, &ev_vec) != -1 && ev_vec != 0) {
                stat = advance_state(&stat, ev_vec->evs, ev_vec->wptr);

            } else {
                // stat should remain being the last updated state
                // if no input is found in frame i, then copy previous
                // state
                stat = advance_state(&stat, NULL, 0);
            }
            vec_set(&state_trace, i, &stat);
        }
        draw_state(game_state.screen.renderer, &stat,
                   game_state.screen.w, game_state.screen.h,32);
        SDL_RenderPresent(game_state.screen.renderer);

        Uint32 game_now = SDL_GetTicks();
        int delay = game_start + 25 * frame - game_now;

        if (delay > 0) {
            SDL_Delay(delay);
        }
        frame++;
        min_frame = frame;
    }

    game_quit();

    return 0;
}
