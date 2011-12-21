package test.frame;

import java.awt.Dimension;
import java.io.IOException;

import javax.swing.JPanel;
import javax.swing.JTabbedPane;

import test.system.SystemPanel;
import test.option.OptionPanel;
import test.plotgraph.PlotGraphPanel;
import test.automaton.AutoMatonPanel;
import test.funcgraph.FuncGraphPanel;
import test.gnuplot.GnuplotPanel;
import test.graph.PhaseViewer;

public class ToolTab extends JTabbedPane {

	public SystemPanel systemPanel;
//	public TreeLayoutDemoPanel treelayoutdemoPanel;
	public PhaseViewer phaseviewer;
	public AutoMatonPanel automatonpanel;
	public FuncGraphPanel funcgraphpanel;
	public PlotGraphPanel plotGraphPanel;
	public GnuplotPanel gnuplotPanel;
	public OptionPanel optionPanel;



	public ToolTab(){
		setMinimumSize(new Dimension(0,0));
		setFocusable(false);

		systemPanel = new SystemPanel();
		addTab("Simulation", systemPanel);

		funcgraphpanel = new FuncGraphPanel();
		addTab("FuncGraph",funcgraphpanel);

		plotGraphPanel = new PlotGraphPanel();
//		addTab("PlotGraph",plotGraphPanel);

		gnuplotPanel = new GnuplotPanel();
		addTab("Gnuplot", gnuplotPanel);



//		String filename = "Nulltext.txt";//"pp_ip_output_test.txt";
		phaseviewer = new PhaseViewer();
//		treelayoutdemo.getFileName(filename);
		addTab("PhaseViewer",phaseviewer);

		automatonpanel = new AutoMatonPanel();
		addTab("AutomatonVisualizer",automatonpanel);

		optionPanel = new OptionPanel();
		addTab("Option", optionPanel);

	}

	public void setTab(String tab){
		if(tab.equals("Simulation")){
			setSelectedComponent(systemPanel);
		}else if(tab.equals("FuncGraph")){
			setSelectedComponent(funcgraphpanel);
		}else if(tab.equals("PhaseViewer")){
			setSelectedComponent(phaseviewer);
		}else if(tab.equals("AutomatonVisualizer")){
			setSelectedComponent(automatonpanel);
		}else if(tab.equals("GnuplotPanel")){
			setSelectedComponent(gnuplotPanel);
		}else if(tab.equals("PlotGraphPanel")){
			setSelectedComponent(plotGraphPanel);
		}
	}

}
