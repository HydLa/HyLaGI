package test.sakuplot;

import javax.swing.UIManager;
import javax.swing.WindowConstants;

import test.sakuplot.gui.MainFrame;

/**
 * <p style="font-size:28pt;">
 * 説明しよう！<br />
 * このクラスはプロットパネルのテストベンチである！<br />
 * プロットパネル周辺の機能を単体で調査したかったらここでテストしよう！<br />
 * </p>
 */
public class PlotTest
{
	public static void main(String[] args)
	{
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		MainFrame frame = new MainFrame();
//		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setVisible(true);
	}
}
