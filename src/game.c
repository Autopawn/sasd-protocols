#include <SDL2/SDL.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define SCREEN_SCALE 1
#define SCREEN_NAME "sasd-protocols"

#include "balls/balls.h"
#include "balls/draw.h"

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

int main(int argc, char* argv[])
{
    game_init();

    // Initial state, one ball is from player 0
    state stat = initial_state(0);

    event event_buffer[8];
    int n_events = 0;

    // Connection event:
    event connect;
    connect.frame = 0;
    connect.summon_frame = 0;
    connect.button = N_BUTTONS;
    connect.pressed = 1;

    for(int i=0;i<3;i++){
        connect.player = i;
        event_buffer[n_events] = connect;
        n_events++;
    }

    SDL_Event event;
    while (game_state.running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                game_state.running = 0;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                event_buffer[n_events].frame = 9001;
                event_buffer[n_events].summon_frame = 9001;
                event_buffer[n_events].player = 0;
                event_buffer[n_events].pressed = (event.type == SDL_KEYDOWN);
                int good = 1;
                switch(event.key.keysym.sym){
                    case SDLK_RIGHT:
                        event_buffer[n_events].button = 0;
                    break;
                    case SDLK_UP:
                        event_buffer[n_events].button = 1;
                    break;
                    case SDLK_LEFT:
                        event_buffer[n_events].button = 2;
                    break;
                    case SDLK_DOWN:
                        event_buffer[n_events].button = 3;
                    break;
                    default:
                        good = 0;
                    break;
                }
                if(good) n_events++;
                break;
            }
        }

        stat = advance_state(&stat,event_buffer,n_events);
        n_events = 0;

        SDL_SetRenderDrawColor(game_state.screen.renderer,127,127,127,127);
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
