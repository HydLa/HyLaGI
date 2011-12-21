package test.robot;

import javax.swing.JFrame;

public class RobotFrame {

	private static boolean isRoboPane;
	JFrame frame;

	public RobotFrame(){
		frame = new JFrame();
		frame.setTitle("Robot");
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.setSize(200, 200);
		frame.setVisible(true);
	}

	public static void setRoboChecker(boolean b) {
		isRoboPane = b;

	}

	public static boolean isRobochecker() {
		return isRoboPane;
	}

}
