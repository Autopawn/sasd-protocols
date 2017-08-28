#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define SCREEN_NAME "sasd-protocols"

#include "balls/balls.h"
#include "balls/draw.h"

#include "packets.h"
#include "recvpool.h"
#include "vec.h"

#include <netutils.h>
#include <balls/balls.h>

static int input_lags[MAX_PLAYERS] = { 0, 200, 0, 0, 0, 0, 0, 0};
static int local_lags[MAX_PLAYERS] = { 0,  0,  0, 0, 0, 0, 0, 0};
// ^ Must try to emulate the reception lag for the server.

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

typedef struct {
	int count;
	int wptr;
	state stats[];
} *st_vec_t;

ev_vec_t ev_vec_create()
{
	ev_vec_t ev_vec = malloc(sizeof(*ev_vec) + 8 * sizeof(event));
	ev_vec->count = 8;
	ev_vec->wptr = 0;
	return ev_vec;
}

st_vec_t st_vec_create()
{
	st_vec_t st_vec = malloc(sizeof(*st_vec) + 8 * sizeof(state));
	st_vec->count = 8;
	st_vec->wptr = 0;
	return st_vec;
}

void ev_vec_destroy(ev_vec_t ev_vec)
{
	free(ev_vec);
}

void st_vec_destroy(ev_vec_t st_vec)
{
	free(st_vec);
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

void st_vec_push(st_vec_t *st_vec_ref, state st)
{
	st_vec_t st_vec = *st_vec_ref;

	if (st_vec->wptr == st_vec->count) {
		st_vec->count += st_vec->count / 2;
		st_vec = *st_vec_ref = realloc(st_vec,
		                               sizeof(*st_vec) + st_vec->count * sizeof(state));
	}

	st_vec->stats[st_vec->wptr++] = st;
}

// CLEAN!
int main(int argc, char* argv[])
{
	game_init();

	// Initial state, one ball is from player 0
	state stat = initial_state(0);

	int trace_size = 512;

    int state_memory_access = 0;
	st_vec_t state_memory = st_vec_create();

	// IMPORTANT: event trace only holds local events!
	auto_vec_t event_trace = vec_create(trace_size, sizeof(ev_vec_t), 1);

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

	event connect;
	connect.frame = frame + local_lags[player];
	connect.summon_frame = frame;
	connect.button = N_BUTTONS;
	connect.pressed = 1;
	connect.player = player;

	ev_vec_t ev_vec = ev_vec_create();
	vec_set(&event_trace, frame, &ev_vec);
	ev_vec_push(&ev_vec, connect);

	send_packet(socket, &connect, EVENT);

	SDL_Event sdl_event;

	Uint32 game_start = SDL_GetTicks();
	while (game_state.running) {
        char title[256];
        sprintf(title, "%s client: %d frame: %d", game_state.screen.name, player,frame);
        SDL_SetWindowTitle(game_state.screen.window, title);

		if (vec_get(event_trace, frame, &ev_vec) == -1 || ev_vec == 0) {
			ev_vec = ev_vec_create();
			vec_set(&event_trace, frame, &ev_vec);
		}

		while (SDL_PollEvent(&sdl_event)) {
			switch (sdl_event.type) {
				case SDL_QUIT:
					game_state.running = 0;
					break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					ev.frame = frame + local_lags[player];
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

		// Send events
		for (int i = 0; i < ev_vec->wptr; i++) {
			// In this case, events are local
			SDL_assert(ev_vec->evs[i].player == player);
			send_packet(socket, ev_vec->evs + i, EVENT);
		}

		// Receive states
		while (recvpool_retrieve(&pool, &packet, 0) != -1) {
			if (packet.ptype == STATE) {
                st_vec_push(&state_memory,packet.payload.state);
			}
		}
        // Load state according to the simulated input lag:
        if(state_memory_access<state_memory->wptr){
            state looked = state_memory->stats[state_memory_access];
            if(frame>=looked.frame+input_lags[player]){
                stat = looked;
                state_memory_access++;
            }
        }

		// Synchronize - Apply all events that have not been applied in server side
		// to improve responsiveness
        printf("%d %d\n",player,stat._lie);
		for (int i = stat.frame; i < frame; i++) {
			if (i - local_lags[player] <= stat._lie) {
				stat = advance_state(&stat, NULL, 0);
			} else {
				if (i - local_lags[player] >= 0 && vec_get(event_trace, i - local_lags[player], &ev_vec) != -1 && ev_vec != 0) {
					stat = advance_state(&stat, ev_vec->evs, ev_vec->wptr);
				} else {
					stat = advance_state(&stat, NULL, 0);
				}
			}
		}

		// Draw
		draw_state(game_state.screen.renderer, &stat,
		           game_state.screen.w, game_state.screen.h,32);
		SDL_RenderPresent(game_state.screen.renderer);

		Uint32 game_now = SDL_GetTicks();
		int delay = game_start + 25 * frame - game_now;

		if (delay > 0) {
			SDL_Delay(delay);
		}
		frame++;
	}

	game_quit();

	return 0;
}
