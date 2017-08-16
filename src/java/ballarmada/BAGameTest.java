package ballarmada;

import javax.swing.JFrame;
import java.awt.EventQueue;

public class BAGameTest extends JFrame{
	public BAGameTest(){
		int nBalls = 4;
		BAState inistate = new BAState(nBalls,9001);
		inistate.balls.add(new BABall(0, BAState.arena_size, inistate.rand));
		//
		BATrace trace = new BATrace(0,inistate);
		//
		setSize(600,600);
		setLocationRelativeTo(null);
		setTitle("Ball Armada - Game Test");
		setResizable(false);
		setVisible(true);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		//
		BAGameplay gameplay = new BAGameplay(trace);
		add(gameplay);
	}
	public static void main(String[] args){
		EventQueue.invokeLater(new Runnable(){
			@Override
			public void run(){
				BAGameTest ex = new BAGameTest();
				ex.setVisible(true);
			}
		});
	}
}
