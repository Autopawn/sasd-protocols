import java.awt.Graphics;
import java.awt.Color;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JPanel;
import javax.swing.Timer;

public class BAGameplay extends JPanel implements KeyListener,ActionListener{
	public static final int delay = 30;
	private Timer clock;
	public BATrace trace;
	public int display_time;

	public BAGameplay(BATrace trace){
		addKeyListener(this);
		setFocusable(true);
		setFocusTraversalKeysEnabled(false);
		//
		clock = new Timer(delay,this);
		clock.start();
		//
		this.trace = trace;
	}

	@Override
	public void paintComponent(Graphics g){
		super.paintComponent(g);
		display(g);
	}

	public void display(Graphics g){
		// Draw state: //TODO: Change time to the right one.
		System.out.println("Getting: "+display_time);
		BAState state = trace.get_state_at(display_time);
		System.out.println("Displaying: "+display_time);
		int sizex = getWidth();
		int sizey = getHeight();
		float scale = Math.min(sizex,sizey)/1.2f/BAState.arena_size;
		int arena_dsize = Math.round(BAState.arena_size*scale);
		g.setColor(Color.black);
		g.fillRect(sizex/2-arena_dsize/2,sizey/2-arena_dsize/2,
			arena_dsize,arena_dsize);
		for(BABall ball:state.balls){
			int ball_dsize = Math.round(BABall.ball_radious*scale);
			int ball_dx = Math.round(ball.px*scale);
			int ball_dy = Math.round(ball.py*scale);
			if(ball.player>=0) g.setColor(Color.red);
			else g.setColor(Color.yellow);
			g.drawOval(sizex/2+ball_dx-ball_dsize,sizey/2+ball_dy-ball_dsize,
				2*ball_dsize+1,2*ball_dsize+1);
		}
		g.dispose();
	}

	@Override
	public void actionPerformed(ActionEvent ev){
		if (ev.getSource() == clock){
			display_time+=1;
		}
		repaint();
	}

	@Override
	public void keyPressed(KeyEvent ev){
		if(ev.getKeyCode()==KeyEvent.VK_RIGHT){

		}
	}

	@Override
	public void keyTyped(KeyEvent ev){
	}

	@Override
	public void keyReleased(KeyEvent ev){
	}
}
