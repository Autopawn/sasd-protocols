#include <SDL2/SDL.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define SCREEN_SCALE 1
#define SCREEN_NAME "sasd-protocols"

#include "balls/balls.h"
#include "balls/draw.h"

#include "vec.h"

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

// CLEAN!
int main(int argc, char* argv[])
{
    game_init();

    // Initial state, one ball is from player 0
    state stat = initial_state(0);
    stat.balls[0].player = 0;

    int trace_size = 512;

    auto_vec_t state_trace = vec_create(trace_size, sizeof(state));
    vec_t* event_trace = malloc(sizeof(vec_t) * trace_size);

    int events_max = 16;

    event event_buffer[8];
    int n_events = 0;

    int frame = 0;

    event ev;

    SDL_Event sdl_event;
    while (game_state.running) {
        if (frame >= trace_size) {
            trace_size += 512;
            event_trace = realloc(event_trace, sizeof(vec_t) * trace_size);
        }
        vec_set(&state_trace, frame, &stat);
        event_trace[frame] = vec_create(16, sizeof(ev));

        while (SDL_PollEvent(&sdl_event)) {
            switch (sdl_event.type) {
            case SDL_QUIT:
                game_state.running = 0;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ev.frame = frame;
                ev.summon_frame = frame;
                ev.player = 0;
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
                    vec_set(&event_trace[frame], n_events++, &ev);
                    n_events++;
                }
                break;
            }
        }

        event* evs = (event*)event_trace[frame]->data;
        stat = advance_state(&stat, evs, n_events);
        n_events = 0;

        SDL_SetRenderDrawColor(game_state.screen.renderer, 127, 127, 127, 255);
        SDL_RenderClear(game_state.screen.renderer);
        draw_state(game_state.screen.renderer, &stat,
            game_state.screen.w, game_state.screen.h);
        SDL_RenderPresent(game_state.screen.renderer);

        // Not good approach for now, use a clock.
        SDL_Delay(25);
    }

    game_quit();

    return 0;
}
