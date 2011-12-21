package test.sakuplot;

import javax.swing.UIManager;
import javax.swing.WindowConstants;

import test.sakuplot.gui.MainFrame;

/**
 * <p style="font-size:28pt;">
 * �������悤�I<br />
 * ���̃N���X�̓v���b�g�p�l���̃e�X�g�x���`�ł���I<br />
 * �v���b�g�p�l�����ӂ̋@�\��P�̂Œ��������������炱���Ńe�X�g���悤�I<br />
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
