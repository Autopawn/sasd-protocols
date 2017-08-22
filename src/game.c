#include <SDL2/SDL.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define SCREEN_SCALE 1
#define SCREEN_NAME "sasd-protocols"

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

    SDL_Event event;
    while (game_state.running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                game_state.running = 0;
                break;
            }
        }

        SDL_RenderClear(game_state.screen.renderer);
        SDL_RenderPresent(game_state.screen.renderer);
    }

    game_quit();

    return 0;
}
