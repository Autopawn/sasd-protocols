#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL.h>

#include "balls.h"

#define BORDER_PIXELS 32

void draw_state(SDL_Renderer* renderer, const state *stat,
        int width, int height);

#endif
