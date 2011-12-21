package test.phaseviewer.gui;

import java.awt.Color;

import org.apache.commons.collections15.functors.ConstantTransformer;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.decorators.PickableEdgePaintTransformer;
import edu.uci.ics.jung.visualization.decorators.PickableVertexPaintTransformer;
import edu.uci.ics.jung.visualization.picking.PickedState;
import edu.uci.ics.jung.visualization.renderers.GradientVertexRenderer;
import test.phaseviewer.graph.CreateGraph;
import test.phaseviewer.graph.DrawGraph;

public class ColorSetter {

	VisualizationViewer<String,Integer> vv;

	ColorSetter(CreateGraph creategraph){
		vv = creategraph.getvv();
	}

	public void setbackground(Color color) {
		vv.setBackground(color);
	}

    public void setNodeColor(Color basedColor,Color normalColor,Color pickedColor){
        //色、いろいろ グラデーションをつけます
        vv.getRenderer().setVertexRenderer(
                new GradientVertexRenderer<String,Integer>(
                                basedColor, normalColor,// Color.pink,
                                basedColor, pickedColor,//Color.green,
                                vv.getPickedVertexState(),
                                false));

           }

    public void setNodeColor(){
    	//塗りつぶすパターン
    	PickedState<String> ps = vv.getPickedVertexState();
    	PickedState<Integer> pes = vv.getPickedEdgeState();
    	vv.getRenderContext().setVertexFillPaintTransformer(new PickableVertexPaintTransformer<String>(ps, Color.pink, Color.green));
    	vv.getRenderContext().setEdgeDrawPaintTransformer(new PickableEdgePaintTransformer<Integer>(pes, Color.black, Color.cyan));
    }
    
	public void setEdgeColor(Color edgeColor) {
    	vv.getRenderContext().setArrowFillPaintTransformer(new ConstantTransformer(edgeColor));
	}



}
