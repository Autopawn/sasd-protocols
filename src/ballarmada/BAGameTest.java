import javax.swing.JFrame;

public class BAGameTest{
	public static void main(String[] args){
		BAState inistate = new BAState(10,9001);
		inistate.balls.add(new BABall(0,BAState.arena_size,inistate.rand));
		//
		BATrace trace = new BATrace(0,inistate);
		//
		JFrame frame = new JFrame();
		frame.setSize(500,500);
		frame.setLocationRelativeTo(null);
		frame.setTitle("Ball Armada - Game Test");
		frame.setResizable(false);
		frame.setVisible(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		//
		BAGameplay gameplay = new BAGameplay(trace);
		frame.add(gameplay);
	}
}
