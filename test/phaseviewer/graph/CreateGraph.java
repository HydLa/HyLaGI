package test.phaseviewer.graph;

import org.apache.commons.collections15.Factory;

import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.graph.Forest;
import edu.uci.ics.jung.visualization.VisualizationViewer;

public interface CreateGraph {
	public boolean createGraph(String fst,String sec);
	public Layout<String,Integer> getLayout();
	public VisualizationViewer<String,Integer> getvv();
	public Forest<String,Integer> getgraph();
	public RadialTreeLayout<String,Integer> getRadialLayout();
}
