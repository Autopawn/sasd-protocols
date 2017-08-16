package ballarmada;

import java.io.Serializable;
import java.lang.Comparable;

public class BAEvent implements Serializable,Comparable<BAEvent>{
	public BAEvent(int player, int button, boolean state){
		this.player = player;
		this.button = button;
		this.state = state;
	}
    int player;
    int button;
    // -1 = Creation of the player.
    // -2 = Destruction of the player.
    // 0 = Right.
    // 1 = Top.
    // 2 = Left.
    // 3 = Down.
    boolean state;
    // 0 = Released.
    // 1 = Pressed.
	@Override
	public int compareTo(BAEvent other) {
		int v1 = player-other.player;
		if(v1!=0) return v1;
		int v2 = button-other.button;
		if(v2!=0) return v2;
		if(state==other.state) return 0;
		else return state?1:-1;
	}
}
