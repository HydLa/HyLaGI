package test.runner.extensiverunner;

import javax.swing.JFrame;

public class ERframe extends JFrame{
	/**
	 *
	 */
	private static final long serialVersionUID = 1L;
	private static boolean isER;
	JFrame frame;

	public ERframe(){
		frame = new JFrame();
		frame.setTitle("Extensive Execution");
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.setSize(200, 200);
		frame.setVisible(true);
	}

	public static void setERchecker(boolean b) {
		isER = b;
	}

	public static boolean isERchecker() {
		return isER;
	}
}
