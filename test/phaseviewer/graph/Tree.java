package test.phaseviewer.graph;

import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.algorithms.layout.TreeLayout;
import edu.uci.ics.jung.graph.DelegateForest;
import edu.uci.ics.jung.graph.Forest;
import edu.uci.ics.jung.visualization.VisualizationViewer;

public class Tree extends LayoutManager{
	Forest<String,Integer> graph = new DelegateForest<String,Integer>();
    TreeLayout<String,Integer> treeLayout = new TreeLayout<String,Integer>(graph);
    RadialTreeLayout<String,Integer> radiallayout = new RadialTreeLayout<String,Integer>(graph);
    VisualizationViewer<String,Integer> vv;
	String treename;

	public Tree(){
		this.vv = getvv(treeLayout);
	}

	public Tree(String treename){
		this.treename = treename;
		this.vv = getvv(treeLayout);
	}

	public VisualizationViewer<String,Integer> getvv(){
		return vv;
	}

	public Forest<String,Integer> getgraph(){
		return graph;
	}

	public TreeLayout<String,Integer> getLayout(){
		return treeLayout;
	}

	public RadialTreeLayout<String,Integer> getRadialLayout(){
		return radiallayout;
	}

	public String getname(){
		return treename;
	}

}
