package test.phaseviewer.gui;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import javax.swing.JComboBox;
import javax.swing.ToolTipManager;

import org.apache.commons.collections15.BidiMap;
import org.apache.commons.collections15.Transformer;
import org.apache.commons.collections15.bidimap.DualHashBidiMap;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.decorators.ToStringLabeller;
import edu.uci.ics.jung.visualization.renderers.Renderer;


public class Tooltips {


	public Tooltips(){

	}

	public void putConstraintTooltip() {
        //制約をポインタ載せたら表示されるようにする
        MainFrame.creategraph.getvv().setVertexToolTipTransformer(new ConstLabeller<String>());
	}



    //tooltipの表示
    private class ConstLabeller<V> extends ToStringLabeller<V>{
    	private ConstLabeller(){
    		init();
    	}
    	public String transform(V v){
			return new TextChanger().changedTooltipText((String) v);
    	}

	   	private void init(){
			ToolTipManager tip = ToolTipManager.sharedInstance();
			tip.setInitialDelay(10);
			tip.setDismissDelay(100000);
		}
    }

}

