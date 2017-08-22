#include "balls.h"

state initial_state(int32_t frame)
{
    state st;
    st.frame = frame;
    for (int i = 0; i < MAX_PLAYERS; i++)
        st.scores[i] = 0;
    int legion_size = 0;
    while (legion_size * legion_size < N_BALLS)
        legion_size++;
    for (int i = 0; i < N_BALLS; i++) {
        st.balls[i].p_x = (1 + 2 * (i % legion_size)) * ARENA_SIZE / (2*legion_size);
        st.balls[i].p_y = (1 + 2 * (i / legion_size)) * ARENA_SIZE / (2*legion_size);
        st.balls[i].v_x = 0;
        st.balls[i].v_y = 0;
        st.balls[i].player = -1;
        for (int r = 0; r < N_BUTTONS; r++)
            st.balls[i].button_states[r] = 0;
    }
    return st;
}

int compare_events(const void* a, const void* b)
{
    const event* aa = (const event*)a;
    const event* bb = (const event*)b;
    int rel;
    rel = aa->frame - bb->frame;
    if (rel != 0)
        return rel;
    rel = aa->player - bb->player;
    if (rel != 0)
        return rel;
    rel = aa->button - bb->button;
    if (rel != 0)
        return rel;
    return (aa->pressed - bb->pressed);
}

int collide_balls(ball* ball_a, ball* ball_b)
{
    //^ Returns the killed ball, 0 if none, -1 if ball_a, +1 if ball_b.
    int64_t delta_x = ball_b->p_x - ball_a->p_x;
    int64_t delta_y = ball_b->p_y - ball_a->p_y;
    // Check if they are colliding:
    if (delta_x * delta_x + delta_y * delta_y < 4 * BALL_RADIOUS * BALL_RADIOUS) {
        // Check if the balls are supposed to kill each other:
        if (ball_a->player >= 0 && ball_b->player >= 0) {
            int64_t spd2_a = ball_a->v_x * ball_a->v_x + ball_a->v_y * ball_a->v_y;
            int64_t spd2_b = ball_b->v_x * ball_b->v_x + ball_b->v_y * ball_b->v_y;
            if (spd2_a > spd2_b)
                return 1;
            else
                return -1;
        } else {
            // ^ Bounce.
            // TODO: Enhance this bouncing to prevent that the speed vector ends pointing to the opposite ball, stucking them.
            // Check the angle of collision and bounce:
            int64_t delta_dif = ABS(delta_x) - ABS(delta_y);
            if (ABS(delta_dif) < BALL_RADIOUS / 3) {
                // Diagonal bouncing:
                int32_t spd_sis_a = ball_a->v_x + ball_a->v_y;
                int32_t spd_tra_a = ball_a->v_x - ball_a->v_y;
                int32_t spd_sis_b = ball_b->v_x + ball_b->v_y;
                int32_t spd_tra_b = ball_b->v_x - ball_b->v_y;
                if (SIGN(delta_x) == SIGN(delta_y)) {
                    int32_t aux = spd_sis_a;
                    spd_sis_a = spd_sis_b;
                    spd_sis_b = aux;
                } else {
                    int32_t aux = spd_tra_a;
                    spd_tra_a = spd_tra_b;
                    spd_tra_b = aux;
                }
                ball_a->v_x = (spd_sis_a + spd_tra_a) / 2;
                ball_a->v_y = (spd_sis_a - spd_tra_a) / 2;
                ball_b->v_x = (spd_sis_b + spd_tra_b) / 2;
                ball_b->v_y = (spd_sis_b - spd_tra_b) / 2;
            } else if (delta_dif > 0) {
                int32_t aux = ball_a->v_x;
                ball_a->v_x = ball_b->v_x;
                ball_b->v_x = aux;
            } else if (delta_dif < 0) {
                int32_t aux = ball_a->v_y;
                ball_a->v_y = ball_b->v_y;
                ball_b->v_y = aux;
            }
        }
    }
    return 0;
}

void reset_ball(ball* ba, int frame)
{
    ba->v_x = 0;
    ba->v_y = 0;
    ba->p_x = (3 + (frame % 3)) * ARENA_SIZE / 9;
    ba->p_y = (3 + ((frame / 3) % 3)) * ARENA_SIZE / 9;
}

state advance_state(const state* ini, event* events, int n_events)
{
    // Sort events so that they work as a set and the function is deterministic.
    qsort(events, n_events, sizeof(event), compare_events);
    // Create state copy:
    state newst = *ini;
    newst.frame += 1;
    // Update the balls:
    for (int i = 0; i < N_BALLS; i++) {
        ball* ba = &newst.balls[i];
        //
        if (ba->player >= 0) {
            // ^ Update player balls:
            // React to events:
            for (int k = 0; k < n_events; k++) {
                event ev = events[k];
                if (ev.frame == newst.frame
                    && ba->player == ev.player
                    && 0 <= ev.button && ev.button < N_BUTTONS) {
                    ba->button_states[ev.button] = ev.pressed;
                }
            }
            // Update ball speed, according to buttons.
            ba->v_x += (ba->button_states[0] ? 1 : 0) * ACCEL;
            ba->v_x -= (ba->button_states[2] ? 1 : 0) * ACCEL;
            ba->v_y += (ba->button_states[3] ? 1 : 0) * ACCEL;
            ba->v_y -= (ba->button_states[1] ? 1 : 0) * ACCEL;
        } else {
            // ^ Update NPC balls:
            // Update speed with dummy AI:
            if (ba->p_x >= 4 * ARENA_SIZE / 5)
                ba->v_x -= ACCEL;
            if (ba->p_x <= ARENA_SIZE / 5)
                ba->v_x += ACCEL;
            if (ba->p_y >= 4 * ARENA_SIZE / 5)
                ba->v_y -= ACCEL;
            if (ba->p_y <= ARENA_SIZE / 5)
                ba->v_y += ACCEL;
        }
        // Limit speed:
        if (ba->v_x > MAX_SPEED)
            ba->v_x = MAX_SPEED;
        if (ba->v_x < -MAX_SPEED)
            ba->v_x = -MAX_SPEED;
        if (ba->v_y > MAX_SPEED)
            ba->v_y = MAX_SPEED;
        if (ba->v_y < -MAX_SPEED)
            ba->v_y = -MAX_SPEED;
        // Update position:
        ba->p_x += ba->v_x;
        ba->p_y += ba->v_y;
        // Reset ball if it falls outside:
        if (ba->p_x < 0 || ba->p_y < 0
            || ba->p_x >= ARENA_SIZE || ba->p_y >= ARENA_SIZE) {
            reset_ball(ba, newst.frame);
            if (ba->player >= 0) {
                newst.scores[ba->player] -= 1;
            }
        }
    }
    // Collide balls:
    for (int a = 0; a < N_BALLS; a++) {
        for (int b = a + 1; b < N_BALLS; b++) {
            int res = collide_balls(&newst.balls[a], &newst.balls[b]);
            if (res == -1) {
                newst.scores[newst.balls[b].player] += 1;
                reset_ball(&newst.balls[a], newst.frame);
            }
            if (res == 1) {
                newst.scores[newst.balls[a].player] += 1;
                reset_ball(&newst.balls[a], newst.frame);
            }
        }
    }
    return newst;
}
