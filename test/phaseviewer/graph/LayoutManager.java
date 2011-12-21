package test.phaseviewer.graph;

import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.visualization.VisualizationViewer;

public abstract class LayoutManager {
    VisualizationViewer<String,Integer> vv;

    public VisualizationViewer<String,Integer> getvv(Layout<String, Integer> layout){
    	vv = new VisualizationViewer<String,Integer>(layout);
    	return vv;
    }

    public abstract VisualizationViewer<String,Integer> getvv();
}
