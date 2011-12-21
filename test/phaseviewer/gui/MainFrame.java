package test.phaseviewer.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JSplitPane;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import test.phaseviewer.graph.CreateGraph;
import test.phaseviewer.graph.CreateTree;
import test.phaseviewer.gui.PvGraphMouseListener;

public class MainFrame extends JFrame{

	private static final long serialVersionUID = 1L;
	//static public JPanel panel = new JPanel();
	RightClick rightclick = new RightClick();
	Tooltips tooltips = new Tooltips();
	static public ControlPanel controlpanel = new ControlPanel();
	static public CreateGraph creategraph;
	private ColorSetter color;
	int height = 600;
	int width = 600;

	public MainFrame(String graphName){
		creategraph = new CreateTree(graphName);
		color = new ColorSetter(creategraph);
		testmethod(creategraph.getvv());
		init(creategraph.getvv());
		setTitle("PhaseViewerTest");
		setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		setSize(width, height);
		add(setPanelvv_controls(creategraph.getvv()), BorderLayout.CENTER);
	}

	private void testmethod(VisualizationViewer<String, Integer> vv){

	}

    private void init(VisualizationViewer<String, Integer> vv){
    	//colorset
    	color.setbackground(Color.white);
		color.setNodeColor(Color.white, Color.pink, Color.green);
		color.setEdgeColor(Color.gray);
		//graphpicked
		setGraphMouseListener();
		//RightClick
		rightclick.set(vv);
		//制約の中身をポインタを載せると表示するようにする
		tooltips.putConstraintTooltip();
    }

    private JSplitPane setPanelvv_controls(VisualizationViewer<String, Integer> vv){
      	JSplitPane jsp;
      	jsp = new JSplitPane(JSplitPane.VERTICAL_SPLIT,vv,controlpanel.set(vv));
      	jsp.setOneTouchExpandable(true);
      	jsp.setResizeWeight(0.9);
		return jsp;
	}

    private void setGraphMouseListener() {
    	creategraph.getvv().addGraphMouseListener(new PvGraphMouseListener<String>());
	}

	public static ControlPanel getControlpanel() {
		return controlpanel;
	}



}
