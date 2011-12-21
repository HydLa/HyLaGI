package test.phaseviewer.gui;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.Map;

import javax.swing.JComboBox;

import org.apache.commons.collections15.BidiMap;
import org.apache.commons.collections15.Transformer;
import org.apache.commons.collections15.bidimap.DualHashBidiMap;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.renderers.Renderer;

public class NodeLabel {

	public void putConstraintLabel(Map<String ,String> Map) {
        //ÉmÅ[ÉhÇÃè„Ç≈ÇÃêßñÒÇÃï\é¶
        MainFrame.creategraph.getvv().getRenderContext().setVertexLabelTransformer(new NodeChanger<String>(Map));
	}

	private class NodeChanger<V> implements Transformer<V,String>{
		BidiMap<String, String> bidiMap;

		private NodeChanger(Map<String, String> Map){
			bidiMap = new DualHashBidiMap<String, String>(Map);
		}

		private String change(V v) {
			String nodename ="";
			nodename = bidiMap.getKey(v).toString();
			return nodename;
		}

		@Override
		public String transform(V v) {
			return change(v);
		}
	}

	public JComboBox setPosBox(final VisualizationViewer<String, Integer> vv){
		JComboBox cb = new JComboBox();
        cb.addItem(Renderer.VertexLabel.Position.N);
        cb.addItem(Renderer.VertexLabel.Position.NE);
        cb.addItem(Renderer.VertexLabel.Position.E);
        cb.addItem(Renderer.VertexLabel.Position.SE);
        cb.addItem(Renderer.VertexLabel.Position.S);
        cb.addItem(Renderer.VertexLabel.Position.SW);
        cb.addItem(Renderer.VertexLabel.Position.W);
        cb.addItem(Renderer.VertexLabel.Position.NW);
        cb.addItem(Renderer.VertexLabel.Position.CNTR);
        cb.addItem(Renderer.VertexLabel.Position.AUTO);
        cb.addItemListener(new ItemListener() {
                        public void itemStateChanged(ItemEvent e) {
                                Renderer.VertexLabel.Position position =
                                        (Renderer.VertexLabel.Position)e.getItem();
                                vv.getRenderer().getVertexLabelRenderer().setPosition(position);
                                vv.repaint();
                        }});
        cb.setSelectedItem(Renderer.VertexLabel.Position.SE);
        return cb;
    }

}
