package test.phaseviewer.graph;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.collections15.BidiMap;
import org.apache.commons.collections15.bidimap.DualHashBidiMap;

import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.algorithms.layout.TreeLayout;
import edu.uci.ics.jung.graph.DelegateForest;
import edu.uci.ics.jung.graph.Forest;
import edu.uci.ics.jung.visualization.decorators.EdgeShape;

import test.phaseviewer.filecontrols.FilePreparation;
import test.phaseviewer.filecontrols.PhaseList;
import test.phaseviewer.gui.MainFrame;

public class DrawGraph {

	CreateGraph creategraph;
	private static Map<String,String> Map;

	private RadialTreeLayout<String,Integer> radialtreeLayout;
	private TreeLayout<String,Integer> treelayout;

	Rings rings;

	public DrawGraph(){
		creategraph = new CreateTree("");
	}

	public DrawGraph(String graphname){
		creategraph = new CreateTree(graphname);
	}

	public Map<String, String> getmap(){
		return Map;
	}

	public RadialTreeLayout<String,Integer> getRadLayout(){
		return radialtreeLayout;
	}

	public TreeLayout<String,Integer> getTreeLayout(){
		return treelayout;
	}


	int i=0;
	public void drawTreeGraph(FilePreparation fp){
		fp.Fileloader();
		Map = fp.getmap();
		Iterator<String> it = Map.keySet().iterator();
        String phase_fst = "";
        String phase_sec = "";
	    while (it.hasNext()) {
	        if(phase_fst.isEmpty()) phase_fst = it.next();
	        phase_sec = it.next();
            if(!Map.get(phase_fst).isEmpty() && !Map.get(phase_sec).isEmpty()){
	           creategraph.createGraph(Map.get(phase_fst), Map.get(phase_sec));
	           //System.out.println(this.toString(phase_fst, phase_sec));
	        }
	        phase_fst = phase_sec;

	    }

	    //Layout‚Í–ˆ‰ñ‚Â‚­‚ç‚È‚¯‚ê‚Î‚È‚ç‚È‚¢
	    treelayout = new TreeLayout<String,Integer>(creategraph.getgraph());
	    radialtreeLayout = new RadialTreeLayout<String,Integer>(creategraph.getgraph());

	    MainFrame.creategraph.getvv().setGraphLayout(treelayout);
        setEdgeShape_Line();
        MainFrame.creategraph.getvv().repaint();
	}

	public void drawTreeGraph(Map<String,String> Map){
		Iterator<String> it = Map.keySet().iterator();
        String phase_fst = "";
        String phase_sec = "";
	    while (it.hasNext()) {
	        if(phase_fst.isEmpty()) phase_fst = it.next();
	        phase_sec = it.next();
            if(!Map.get(phase_fst).isEmpty() && !Map.get(phase_sec).isEmpty()){
	           creategraph.createGraph(Map.get(phase_fst), Map.get(phase_sec));
	        }
	        phase_fst = phase_sec;

	    }

	    //Layout‚Í–ˆ‰ñ‚Â‚­‚ç‚È‚¯‚ê‚Î‚È‚ç‚È‚¢
	    treelayout = new TreeLayout<String,Integer>(creategraph.getgraph());
	    radialtreeLayout = new RadialTreeLayout<String,Integer>(creategraph.getgraph());


        MainFrame.creategraph.getvv().setGraphLayout(new TreeLayout<String,Integer>(creategraph.getgraph()));
        setEdgeShape_Line();
        MainFrame.creategraph.getvv().repaint();
	}

	@SuppressWarnings("rawtypes")
	private void setEdgeShape_Line() {
    	MainFrame.creategraph.getvv().getRenderContext().setEdgeShapeTransformer(new EdgeShape.Line());
	}

	public CreateGraph getCreategraph() {
		return creategraph;
	}

	private String toString(String phase_fst,String phase_sec){
		return ("Map.get(phase_fst) : "+Map.get(phase_fst)+"Map.get(phase_sec) : "+ Map.get(phase_sec));
	}

}
