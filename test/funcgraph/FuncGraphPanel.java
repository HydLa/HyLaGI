package test.funcgraph;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.UIManager;
import javax.swing.WindowConstants;

import test.Env;
import test.graph.PhaseViewer;
import test.sakuplot.gui.MainFrame;
import test.sakuplot.gui.PlotPanel;
import test.sakuplot.model.CompoundGraph;
import test.sakuplot.model.SimulationCase;
import test.sakuplot.model.SimulationCaseParser;

public class FuncGraphPanel extends JPanel{

	private static boolean OnAnotherPanel;
//	private FileToString fts = new FileToString();
	public FuncGraphRunner funcGraphRunner;
	Thread thread;
	CreateNewFrame cnp;
	static MainFrame frame = new MainFrame();

//	PlotPanel panel;
	String filename;

		public FuncGraphPanel(){
			setLayout(new BorderLayout());
//			funcGraphRunner = new FuncGraphRunner();
			try
			{
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
			frame.setVisible(true);
			add(frame, BorderLayout.CENTER);
		}

		public void restart(){
			if(OnAnotherPanel){
				cnp = new CreateNewFrame();
				cnp.setFileName(filename);
				cnp.start();
			}else{
				start();
			}
		}

		public static void setOnAnotherPanel(boolean b){
			OnAnotherPanel = b?true:false;
		}

		public static boolean isOnAnotherPanel(){
			return OnAnotherPanel;
		}

		@SuppressWarnings("deprecation")
		public void kill(){
			funcGraphRunner.stop();
//			thread.stop();
		}

		private void start(){
			if(funcGraphRunner==null){
				funcGraphRunner = new FuncGraphRunner(filename);
			}else{
				funcGraphRunner.setFilename(filename);
			}
			thread = new Thread(funcGraphRunner);
			thread.start();

//			String str;
//			try {
//				SimulationCaseParser scp = new SimulationCaseParser();
//				str = fts.fileToString(new File(filename));
//				String x_axis = Env.get("FUNC_X_AXIS");
//				String y_axis = Env.get("FUNC_Y_AXIS");
//				if (!str.isEmpty()){
//					List<SimulationCase> cases = scp.parseCases(str);
//					List<CompoundGraph> graphList = new ArrayList<CompoundGraph>();
//					for (SimulationCase c : cases)
//					{
//						graphList.add(CompoundGraph.createFromSimulation(c, x_axis, y_axis));
//					}
//					frame.doPlot(graphList);
//				}
//			} catch (IOException e) {
//				// TODO é©ìÆê∂ê¨Ç≥ÇÍÇΩ catch ÉuÉçÉbÉN
//				e.printStackTrace();
//			}
		}




		private void repaintPanel(){
			validate();
		}

		private void clearPanel(){
			removeAll();
		}

		public void setFileName(String filename){
			this.filename = filename;
		}

		private class CreateNewFrame {
			FuncGraphPanel fgp = new FuncGraphPanel();
			CreateNewFrame(){
				JFrame frame = new JFrame();
				int height = Env.getInt("WINDOW_HEIGHT")*8/10;
				int width = Env.getInt("WINDOW_WIDTH")*8/10;
				frame.setTitle("FuncGraph");
				frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
				frame.setSize(width, height);
				frame.add(fgp, BorderLayout.CENTER);
				frame.setVisible(true);
			}

			void start(){
				fgp.start();
			}

			void setFileName(String filename){
				fgp.setFileName(filename);
			}

			void clearFrame(){
				fgp.removeAll();
			}
		}
}





