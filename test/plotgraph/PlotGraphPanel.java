package test.plotgraph;

import java.awt.BorderLayout;
import java.awt.FlowLayout;

import javax.swing.JPanel;

public class PlotGraphPanel extends JPanel{
		private JPanel superpanel = new JPanel(new FlowLayout());
		MainFrame plotframe;

		public PlotGraphPanel(){
			setLayout(new BorderLayout());
			String filename = "balltest.txt";//"plotText.txt";
			start(filename);
			}

		public void restart(String filename){
			clearPanel();
			start(filename);
			repaintPanel();
		}

		private void clearPanel(){
			superpanel.removeAll();
		}

		private void start(String filename){
			plotframe = new MainFrame(filename);
			superpanel.add(plotframe);
			add(superpanel, BorderLayout.CENTER);
		}

		private void repaintPanel(){
			superpanel.validate();
		}
}
