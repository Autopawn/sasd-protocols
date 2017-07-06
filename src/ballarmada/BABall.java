import java.io.Serializable;

import java.lang.Integer;
import java.util.Random;
import java.util.TreeSet;

public class BABall implements Serializable{
    public static final int accel = 2;
    public static final int max_speed = 64;
    public static final int ball_radious = 32;
    public static final int buttons = 4;

    public int player; // -1 indicates yellow ball.
    public int px;
    public int py;
    int vx;
    int vy;
    boolean button_states[];

    public BABall(int player, int arena_size, Random rand){
        this.player = player;
        this.reset(arena_size,rand);
		if(player>=0) button_states = new boolean[buttons];
    }

    void update(int arena_size, final TreeSet<BAEvent> events){
		if(player>=0){
	        // React to events:
	        for(BAEvent ev:events){
	            if(ev.player == player && 0<=ev.button && ev.button<buttons){
	                button_states[ev.button] = ev.state;
	            }
	        }
	        // Update speed, according to buttons.
	        vx += (button_states[0]?1:0)*accel;
	        vx -= (button_states[2]?1:0)*accel;
			vy += (button_states[3]?1:0)*accel;
			vy -= (button_states[1]?1:0)*accel;
		}else{
			// Update speed, according with dummy AI:
			if(px>=arena_size/3) vx-= accel;
			if(px<=-arena_size/3) vx+= accel;
			if(py>=arena_size/3) vy-= accel;
			if(py<=-arena_size/3) vy+= accel;
		}
		// Limit speed:
        if(Math.abs(vx)>max_speed) vx = Integer.signum(vx)*max_speed;
        if(Math.abs(vy)>max_speed) vy = Integer.signum(vy)*max_speed;
        // Update position:
        px += vx;
        py += vy;
    }

    static int collide(BABall ball_a, BABall ball_b){
        //^ Returns the killed player, or -1 if none.
        int delta_x = ball_b.px - ball_a.px;
        int delta_y = ball_b.py - ball_a.py;
        // Check if they are colliding:
        if(delta_x*delta_x+delta_y*delta_y<4*ball_radious*ball_radious){
            // Check if the balls are supposed to kill each other:
            if(ball_a.player>=0 && ball_b.player>=0){
                int spd2_a = ball_a.vx*ball_a.vx+ball_a.vy*ball_a.vy;
                int spd2_b = ball_b.vx*ball_b.vx+ball_b.vy*ball_b.vy;
                if(spd2_a>spd2_b) return ball_b.player;
                else return ball_a.player;
            }else{ //Bounce:
                // Check the angle of collision and bounce:
                int delta_dif = Math.abs(delta_x)-Math.abs(delta_y);
                if(Math.abs(delta_dif)<ball_radious/3){
                    // Diagonal bouncing:
                    int spd_sis_a = ball_a.vx+ball_a.vy;
                    int spd_tra_a = ball_a.vx-ball_a.vy;
                    int spd_sis_b = ball_b.vx+ball_b.vy;
                    int spd_tra_b = ball_b.vx-ball_b.vy;
                    if(Integer.signum(delta_x)==Integer.signum(delta_y)){
                        int aux = spd_sis_a;
                        spd_sis_a = spd_sis_b;
                        spd_sis_b = aux;
                    }else{
                        int aux = spd_tra_a;
                        spd_tra_a = spd_tra_b;
                        spd_tra_b = aux;
                    }
                    ball_a.vx = (spd_sis_a+spd_tra_a)/2;
                    ball_a.vy = (spd_sis_a-spd_tra_a)/2;
                    ball_b.vx = (spd_sis_b+spd_tra_b)/2;
                    ball_b.vy = (spd_sis_b-spd_tra_b)/2;
                }else if(delta_dif>0){
                    int aux = ball_a.vx;
                    ball_a.vx = ball_b.vx;
                    ball_b.vx = aux;
                }else if(delta_dif<0){
                    int aux = ball_a.vy;
                    ball_a.vy = ball_b.vy;
                    ball_b.vy = aux;
                }
            }
        }
        return -1;
    }

    void reset(int arena_size, Random rand){
        boolean center = (player>=0);
        px = 0;
        py = 0;
        while(center != (Math.abs(px)<arena_size/3 && Math.abs(py)<arena_size/3)){
            px = rand.nextInt(arena_size)-arena_size/2;
            py = rand.nextInt(arena_size)-arena_size/2;
        }
        vx = 0;
        vy = 0;
    }
}
