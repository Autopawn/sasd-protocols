#ifndef BALLS_H
#define BALLS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define SIGN(X) ((X)==0?0:((X)>0)?1:-1)
#define ABS(X) ((X)<0?-(X):(X))

#define MAX_PLAYERS 8
#define N_BALLS 12
#define ARENA_SIZE 3000

#define ACCEL 2
#define MAX_SPEED 64
#define BALL_RADIOUS 56
#define N_BUTTONS 4

typedef struct {
    int32_t p_x, p_y;
    int32_t v_x, v_y;
    int32_t player; // -1 indicates NPC.
    char button_states[N_BUTTONS];
	int _sent;
} ball;

typedef struct {
    char paused;
    int32_t frame;
    int32_t death_count;
    int32_t scores[MAX_PLAYERS];
    ball balls[N_BALLS];
	// Last Influential Event (dead reckoning specific)
	int32_t _lie;
} state;

typedef struct {
    int32_t frame;
    int32_t summon_frame;
    int32_t player;
    char button; // button=N_BUTTONS means connect/disconnect
    char pressed;
} event;

state initial_state(int32_t frame);

// Función determinista que avanza un estado a partir de un conjunto de eventos.
state advance_state(const state *ini, event *events, int n_events);

#endif
