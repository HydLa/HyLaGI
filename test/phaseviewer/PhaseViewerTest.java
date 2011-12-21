package test.phaseviewer;

import java.awt.BorderLayout;
import java.awt.Color;

import javax.swing.JFrame;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import test.Env;
import test.graph.PhaseViewer;
import test.phaseviewer.filecontrols.FilePreparation;
import test.phaseviewer.filecontrols.PhaseList;
import test.phaseviewer.graph.CreateTree;
import test.phaseviewer.graph.DrawGraph;
import test.phaseviewer.gui.MainFrame;

public class PhaseViewerTest {

	public static void main(String[] args) {

		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}

		MainFrame frame = new MainFrame("‚±‚±‚ÉƒOƒ‰ƒt‚Ì–¼‘O‚ð“ü‚ê‚æ‚¤");
		frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		frame.setVisible(true);
	}
}
