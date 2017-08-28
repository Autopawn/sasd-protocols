#include "draw.h"

static const int colors[][3] = {
    {0x00,0x00,0x00},
    {0x00,0x00,0xFF},
    {0x00,0xFF,0x00},
    {0x00,0xFF,0xFF},
    {0xFF,0x00,0x00},
    {0xFF,0x00,0xFF},
    {0xFF,0xFF,0x00},
    {0x7F,0x7F,0xFF},
    {0x7F,0xFF,0x7F},
    {0xFF,0x7F,0x7F},
};
static const int n_colors = 10;

void draw_state(SDL_Renderer* renderer, const state *stat,
        int width, int height, unsigned char alpha){
    int space, offset_x, offset_y;
    if(width>=height){
        space = height-2*BORDER_PIXELS;
        offset_x = (width-space)/2;
        offset_y = BORDER_PIXELS;
    }else{
        space = width-2*BORDER_PIXELS;
        offset_x = BORDER_PIXELS;
        offset_y = (height-space)/2;
    }
    int ball_radious = BALL_RADIOUS*space/ARENA_SIZE;
    //
    SDL_Rect arena = {offset_x, offset_y, space, space};
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,127,127,127,alpha);
    SDL_RenderFillRect(renderer,NULL);
    SDL_SetRenderDrawColor(renderer,143,143,143,alpha);
    SDL_RenderFillRect(renderer,&arena);
    SDL_SetRenderDrawColor(renderer,0,0,0,alpha);
    SDL_RenderDrawRect(renderer,&arena);

    int n_scores = 0;
    // Draw balls.
    for(int i=0;i<N_BALLS;i++){
        int pos_x = offset_x+(stat->balls[i].p_x*space+ARENA_SIZE/2)/ARENA_SIZE;
        int pos_y = offset_y+(stat->balls[i].p_y*space+ARENA_SIZE/2)/ARENA_SIZE;
        int color;
        if(stat->balls[i].player>=0){
            color = 1+(stat->balls[i].player%(n_colors-1));
        }else{
            color = 0;
        }
        SDL_SetRenderDrawColor(renderer,
            colors[color][0],colors[color][1],colors[color][2],255);
        // This is a circle in my heart.
        SDL_Rect circle = {pos_x-ball_radious+1, pos_y-ball_radious+1,
            2*ball_radious-1,2*ball_radious-1};
        SDL_Rect smaller_circle = {pos_x-ball_radious+3, pos_y-ball_radious+3,
            2*ball_radious-5,2*ball_radious-5};
        int inner_x=0, inner_y=0;
        inner_x += stat->balls[i].button_states[0];
        inner_x -= stat->balls[i].button_states[2];
        inner_y -= stat->balls[i].button_states[1];
        inner_y += stat->balls[i].button_states[3];
        smaller_circle.x += ball_radious/2*inner_x;
        smaller_circle.y += ball_radious/2*inner_y;
        SDL_RenderDrawRect(renderer,&circle);
        SDL_RenderDrawRect(renderer,&smaller_circle);
        if(stat->balls[i].player>=0){
            for(int k=0;k<stat->scores[stat->balls[i].player];k++){
                SDL_Rect scor = {1+(k+(k/5))*(SCORE_MARK_SIZE-3),
                    1+SCORE_MARK_SIZE*n_scores,
                    SCORE_MARK_SIZE-5,SCORE_MARK_SIZE-2};
                SDL_SetRenderDrawColor(renderer,
                    colors[color][0],colors[color][1],colors[color][2],255);
                SDL_RenderFillRect(renderer,&scor);
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderDrawRect(renderer,&scor);
            }
            n_scores++;
        }
    }
}
