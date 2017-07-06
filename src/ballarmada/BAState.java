import java.io.ByteArrayInputStream;
import java.io.ObjectInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectOutputStream;

import java.io.Serializable;

import java.util.Random;
import java.util.TreeSet;
import java.util.LinkedList;

public class BAState implements Serializable{
    public static final int arena_size = 1000;

    LinkedList<BABall> balls;
    Random rand;

    public BAState(int npc_balls, int random_seed){
        // Setup random number generator:
        rand = new Random(random_seed);
        // Create random balls
        balls = new LinkedList<BABall>();
        for(int i=0;i<npc_balls;i++){
            BABall ball = new BABall(-1,arena_size,rand);
            balls.add(ball);
        }
    }

    public BAState copy(){
        // Create deep copy of the state.
        try{
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            ObjectOutputStream oos = new ObjectOutputStream(bos);
            oos.writeObject(this);
            oos.flush();
            ByteArrayInputStream bin = new ByteArrayInputStream(bos.toByteArray());
            ObjectInputStream ois = new ObjectInputStream(bin);
            BAState state_copy = (BAState) ois.readObject();
            return state_copy;
        }catch(Exception ex){
            ex.printStackTrace();
        }
        return null;
    }

    public final BAState update(final TreeSet<BAEvent> events){
        // Create a copy of the state:
        BAState next = this.copy();
        // Creation of balls:
        for(BAEvent ev:events){
            if(ev.button==-1){
                // Create a ball for the given player.
                BABall ball = new BABall(ev.player,arena_size,rand);
                next.balls.add(ball);
            }
        }
        // Update balls:
        for(BABall ball:next.balls){
            ball.update(arena_size,events);
        }
        // Collide balls:
        LinkedList<Integer> dead_balls = new LinkedList<Integer>();
        for(int i=0;i<next.balls.size();i++){
            for(int j=i+1;j<next.balls.size();j++){
                int dead_ball = BABall.collide(next.balls.get(i),next.balls.get(j));
                if(dead_ball>=0) dead_balls.add(dead_ball);
            }
        }
        // Destruction of balls due events:
        for(BAEvent ev:events){
            if(ev.button==-2){
                for(int i=0;i<next.balls.size();i++){
                    if(next.balls.get(i).player==ev.player) next.balls.remove(i);
                    break;
                }
            }
        }
        // Reset of balls due lost collisions:
        for(int k:dead_balls){
            for(int i=0;i<next.balls.size();i++){
                if(next.balls.get(i).player==k) next.balls.get(i).reset(arena_size,rand);
                break;
            }
        }
        return next;
    }
}
