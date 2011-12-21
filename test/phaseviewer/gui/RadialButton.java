package test.phaseviewer.gui;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.JToggleButton;

import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.layout.RadialTreeLayout;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.layout.LayoutTransition;
import edu.uci.ics.jung.visualization.util.Animator;

public class RadialButton {

	 LayoutTransition<String,Integer> lt;
	 RadialTreeLayout<String,Integer> radialLayout;

	 Layout<String,Integer> treeLayout;

	public RadialButton() {
		super();

	}


    public JToggleButton set( final VisualizationViewer<String, Integer> vv){
        JToggleButton radial = new JToggleButton("Radial");
        radial.addItemListener(new ItemListener() {
        	public void itemStateChanged(ItemEvent e) {
                radialLayout = MainFrame.controlpanel.getStartbutton().getDrawgraph().getRadLayout();
                treeLayout = MainFrame.controlpanel.getStartbutton().getDrawgraph().getTreeLayout();
                if(e.getStateChange() == ItemEvent.SELECTED) {
                	//LayoutTransition<String,Integer>
                	lt =  new LayoutTransition<String,Integer>(vv,treeLayout, radialLayout);
                    Animator animator = new Animator(lt);
                    animator.start();
                    vv.getRenderContext().getMultiLayerTransformer().setToIdentity();
                    vv.addPreRenderPaintable(MainFrame.controlpanel.getStartbutton().getRings());
                 } else {
                    //LayoutTransition<String,Integer>
                    lt =  new LayoutTransition<String,Integer>(vv, radialLayout, treeLayout);
                    Animator animator = new Animator(lt);
                    animator.start();
                    vv.getRenderContext().getMultiLayerTransformer().setToIdentity();
                    vv.removePreRenderPaintable(MainFrame.controlpanel.getStartbutton().getRings());
                 }
                 vv.repaint();
           	}
        }
        );
    	return radial;
    }

}
