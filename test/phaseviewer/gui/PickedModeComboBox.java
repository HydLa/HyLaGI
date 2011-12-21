package test.phaseviewer.gui;

import javax.swing.JComboBox;

import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.DefaultModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;

public class PickedModeComboBox {

	public JComboBox set(VisualizationViewer<String,Integer> vv) {
        //パネル中の操作
        DefaultModalGraphMouse<String, String> graphMouse = new DefaultModalGraphMouse<String, String>();
        vv.setGraphMouse(graphMouse);
        //パネル中の操作アイコンの変更
        JComboBox modeBox = graphMouse.getModeComboBox();
        modeBox.addItemListener(graphMouse.getModeListener());
        graphMouse.setMode(ModalGraphMouse.Mode.TRANSFORMING);
		return modeBox;
	}

}
